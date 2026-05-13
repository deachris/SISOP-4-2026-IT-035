#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUFFER_SIZE 1024

void handle_connection(int sock) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    printf("Connected to DB Server on port %d\n", PORT);
    printf("Type HELP for available commands\n");
    printf("Type EXIT to quit\n\n");

    while (1) {
        printf("db > ");
        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "EXIT") == 0) break;
        if (strlen(buffer) == 0) continue;

        send(sock, buffer, strlen(buffer), 0);

        int valread = read(sock, response, BUFFER_SIZE);
        if (valread > 0) {
            response[valread] = '\0';
            printf("%s\n", response);
        }
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error saat membuat socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n Alamat tidak valid \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Koneksi Gagal. Pastikan container db_app sudah berjalan. \n");
        return -1;
    }

    handle_connection(sock);
    close(sock);

    return 0;
}
