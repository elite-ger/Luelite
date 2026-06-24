#include "luelib_http.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #define closesocket close
#endif

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} HttpResponse;

static HttpResponse* response_create(void) {
    HttpResponse *r = (HttpResponse*)malloc(sizeof(HttpResponse));
    r->data = NULL;
    r->size = 0;
    r->capacity = 0;
    return r;
}

static void response_append(HttpResponse *r, const char *data, size_t len) {
    if (r->size + len > r->capacity) {
        r->capacity = r->capacity ? r->capacity * 2 : 4096;
        r->data = (char*)realloc(r->data, r->capacity + 1);
    }
    memcpy(r->data + r->size, data, len);
    r->size += len;
    r->data[r->size] = '\0';
}

static void response_free(HttpResponse *r) {
    if (r->data) free(r->data);
    free(r);
}

static char* http_extract_body(const char *response, size_t *body_len) {
    const char *sep = strstr(response, "\r\n\r\n");
    if (!sep) {
        *body_len = 0;
        return NULL;
    }
    *body_len = strlen(sep + 4);
    return (char*)(sep + 4);
}

static int http_get_raw(const char *host, int port, const char *path, HttpResponse *r) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct hostent *server = gethostbyname(host);
    if (!server) {
        closesocket(sock);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
    addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        closesocket(sock);
        return -1;
    }

    char request[2048];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "User-Agent: Luelite/0.1\r\n"
        "\r\n",
        path, host);

    if (send(sock, request, strlen(request), 0) < 0) {
        closesocket(sock);
        return -1;
    }

    char buffer[4096];
    int n;
    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        response_append(r, buffer, n);
    }

    closesocket(sock);
    return 0;
}

static int parse_url(const char *url, char *host, size_t host_len, 
                     int *port, char *path, size_t path_len) {
    const char *start;
    if (strncmp(url, "http://", 7) == 0) {
        start = url + 7;
        *port = 80;
    } else {
        start = url;
        *port = 80;
    }

    const char *slash = strchr(start, '/');
    const char *colon = strchr(start, ':');

    if (colon && (!slash || colon < slash)) {
        size_t host_part = colon - start;
        if (host_part >= host_len) return -1;
        memcpy(host, start, host_part);
        host[host_part] = '\0';
        *port = atoi(colon + 1);
        if (slash) {
            strncpy(path, slash, path_len);
        } else {
            strcpy(path, "/");
        }
    } else if (slash) {
        size_t host_part = slash - start;
        if (host_part >= host_len) return -1;
        memcpy(host, start, host_part);
        host[host_part] = '\0';
        strncpy(path, slash, path_len);
    } else {
        strncpy(host, start, host_len);
        strcpy(path, "/");
    }
    return 0;
}


static int l_http_get(lua_State *L) {
    const char *url = luaL_checkstring(L, 1);
    
    char host[256];
    char path[1024];
    int port;

    if (parse_url(url, host, sizeof(host), &port, path, sizeof(path)) != 0) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid URL");
        return 2;
    }

#ifdef _WIN32
    static int winsock_init = 0;
    if (!winsock_init) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        winsock_init = 1;
    }
#endif

    HttpResponse *response = response_create();
    if (http_get_raw(host, port, path, response) != 0) {
        response_free(response);
        lua_pushnil(L);
        lua_pushstring(L, "Connection failed");
        return 2;
    }

    size_t body_len;
    char *body = http_extract_body(response->data, &body_len);
    
    if (body) {
        lua_pushlstring(L, body, body_len);
    } else {
        lua_pushstring(L, response->data);
    }
    
    response_free(response);
    return 1;
}

static int l_http_post(lua_State *L) {
    const char *url = luaL_checkstring(L, 1);
    const char *body_data = luaL_checkstring(L, 2);
    
    lua_pushnil(L);
    lua_pushstring(L, "POST not implemented yet");
    return 2;
}


static const luaL_Reg http_lib[] = {
    {"get",  l_http_get},
    {"post", l_http_post},
    {NULL, NULL}
};

int luaopen_luelib_http(lua_State *L) {
    luaL_newlib(L, http_lib);
    return 1;
}