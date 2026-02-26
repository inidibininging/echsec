#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

char* string_join(char** strings, int num_strings, char separator) {
    size_t total_length = 0; // Calculate the total length of the joined string.
    for (int i = 0; i < num_strings; i++) {
        total_length += strlen(strings[i]) + 1; // Add the length of each string and a separator.
    }
    total_length += num_strings - 1; // Subtract one to avoid adding an extra separator at the end.
    char* joined = malloc(total_length + 1); // Allocate memory for the joined string.
    int pos = 0; // Position in the joined string.
    for (int i = 0; i < num_strings; i++) {
        size_t len = strlen(strings[i]);
        memcpy(&joined[pos], strings[i], len); // Copy each string into the joined buffer.
        pos += len; // Move to the next position in the buffer.
        if (i < num_strings - 1) { // Add a separator between strings, except for the last one.
            joined[pos++] = separator;
        }
    }
    joined[pos] = '\0'; // Add a null terminator to the end of the string.
    return joined; // Return the joined string.
}

char* string_concatenate(const char* str1, const char* str2) {
    const int s1 = strlen(str1);
    const int s2 = strlen(str2);
    char* result = malloc(s1 + s2 + 1); // Allocate memory for the result string.
    //strcpy(result, str1); // Copy the first string into the result buffer.
    if(result) {
     memcpy(result, str1, s1);
    }

    strcat(result, str2); // Append the second string to the result buffer.
    return result; // Return the concatenated string.
}


void handle_request(int sock) {
    char buffer[1024];
    int bytes_read, bytes_written;
    char filename[1024];
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    getpeername(sock, (struct sockaddr*)&client_address, &client_address_size);
    strcpy(filename, inet_ntoa(client_address.sin_addr)); // extract IP address and use it as filename prefix
    //strcat(filename, "html"); // append ".html" extension
    int fi = 0;
    while(filename[fi] != '\0') {
        if(filename[fi] == '.') {
            filename[fi] = '-';
        }
        fi++;
    }
    printf("%s\n", filename);
/*
    FILE* file = fopen(filename, "w");

    if (!file) {
        printf("Error: could not create file %s\n", filename);
        return;
    }
    
    while ((bytes_read = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, file);
        if (bytes_written != bytes_read) {
            printf("Error: could not write to file %s\n", filename);
            fclose(file);
            return;
        }
    }
    if (bytes_read == 0) {
        printf("Client disconnected\n");
    } else {
        printf("Error: could not receive data from client\n");
    }

    fclose(file);
*/
    
    const char html_content[] = "<html><head><title>echsec</title></head><body>Hello, world!</body></html>";
    const int content_length = strlen(html_content);
    const char response[] = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: ";
    int msgsize = strlen(html_content) + strlen(response);
    char ms[12];
    memset(ms, 0, 12);
    snprintf(ms, sizeof(ms),"%d", msgsize);
    msgsize += strlen(ms);

    char* r = string_concatenate(response, ms);
    
    //sprintf(msgsize, "%d\r\n\r\n", content_length);
    send(sock, r, strlen(r), 0);
    free(r);
    send(sock, html_content, content_length, 0);
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error: could not create socket\n");
        return 1;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("Error: could not bind socket\n");
        return 1;
    }
    listen(sock, 5);
    while (1) {
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock < 0) {
            printf("Error: could not accept connection\n");
            continue;
        }
        handle_request(client_sock);
        //close(client_sock);
    }
    return 0;
}

