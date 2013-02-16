#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include "mp3split.h"
#include "mp3stream.h"
#include <errno.h>

#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>

#define IPV6 0

#if defined(__GNUC__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

int main(int argc, char *argv[] UNUSED) {
    if (argc < 1) {
        //fprintf(stderr, "NO U >:V giev host and port\n");
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);
    // Use this to use a server socket for connections
    /*
    
    int mp3_stream, mp3_server_port;
    struct sockaddr_in mp3_server_addr;
    struct hostent* mp3_server;
    
    mp3_server_port = atoi(argv[2]);
    mp3_stream = socket(AF_INET, SOCK_STREAM, 0);
    if (mp3_stream < 0) {
        //fprintf(stderr, "Error can't into sockets %s\n", strerror(errno));
        exit(1);
    }
    
    mp3_server = gethostbyname(argv[1]);
    if (mp3_server == NULL) {
        //fprintf(stderr, "Error can't into servers %s\n", strerror(errno));
        exit(1);
    }
    
    memset(&mp3_server_addr, 0, sizeof(mp3_server_addr));
    mp3_server_addr.sin_family = AF_INET;
    memcpy(&mp3_server_addr.sin_addr.s_addr, mp3_server->h_addr,
                                                        mp3_server->h_length);
    mp3_server_addr.sin_port = htons(mp3_server_port);
    if (connect(mp3_stream, (struct sockaddr *) &mp3_server_addr,
                    sizeof(mp3_server_addr)) < 0) {
        //fprintf(stderr, "Error can't into connect %s\n", strerror(errno));
        exit(1);
    }
    
    ssize_t n;
    n = write(mp3_stream, "GET / HTTP/1.1\r\n\r\n", 18);
    if (n < 0) {
        //fprintf(stderr, "Error can't into write %s\n", strerror(errno));
        exit(1);
    }
    */
    
    //struct Client* head_client = NULL;
    
    // use this to test stuff
    //int mpd_test = open("mpd_test.mp3", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    struct Client* head_client = malloc(sizeof(struct Client));
    
    head_client->sock = -1;
    head_client->prev = NULL;
    head_client->next = NULL; 
    
    int server, client, port = 8080;
    socklen_t client_len;
    uint8_t buffer[BUFSIZE];
    int af = AF_INET;
    struct sockaddr_storage client_addr;
#if defined(AF_INET6)
    struct addrinfo hints, *res;
    char port_str[5];
    int error = 0;
    if (IPV6)
        af = AF_INET6;
#endif
    server = socket(af, SOCK_STREAM, 0);
    int on = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (server < 0) {
        //fprintf(stderr, "Something went wrong while setting up the server!\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(port_str, sizeof(port_str), "%d", port);

    error = getaddrinfo(NULL, port_str, &hints, &res);
    if (error) {
        perror(gai_strerror(error));
        exit(1);
    }
    
    for(struct addrinfo *iter = res; iter; iter = iter->ai_next) {
        if (iter->ai_family == af) {
            if (bind(server, iter->ai_addr,
                                iter->ai_addrlen) < 0) {
                fprintf(stderr, "Something went wrong while binding the server! %s\n",
                                strerror(errno));
                exit(1);
            }
        }
    }
    freeaddrinfo(res);
    res = NULL;
    
    
    /*listen(server, 10);
    client_len = sizeof(client_addr);
    client = accept(server, (struct sockaddr*) &client_addr, &client_len);
    if (client < 0) {
        //fprintf(stderr, "Something went wrong while accepting the client!\n");
        exit(1);
    }
    head_client->sock = client;
    read(head_client->sock, buffer, BUFSIZE); // lawl we don't really care...
    write(head_client->sock, "HTTP/1.1 200 OK\r\n\r\n", 19); */
    
    // establish shared memory
    
    // actually, fuck shared mem, we're doing it in one process!
    /*
    size_t region_size = sysconf(_SC_PAGE_SIZE);
    char *memname = "/clients";
    
    int shmfd = shm_open(memname, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (shmfd == -1) {
        //fprintf(stderr, "Could not open shared memory\n");
        exit(1);
    }
    
    int r = ftruncate(shmfd, region_size);
    if (r != 0) {
        //fprintf(stderr, "Could not truncate shared memory\n");
        exit(1);
    }
    
    void *ptr = mmap(0, region_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (ptr == MAP_FAILED) {
        //fprintf(stderr, "Map failed\n");
        exit(1);
    }
    close(shmfd);
    
    memcpy(ptr, head_client, sizeof(struct Client));
    free(head_client);
    
    
    pid_t child;
    child = fork();
    if (child == 0) {
        //fprintf(stderr, "mp3_stream fd: %d\n", mp3_stream);
        serve_stream(mp3_stream, ptr);
    }
    */
    
    struct FrameBufferElement* FB_head = malloc(sizeof(
                                          struct FrameBufferElement));
    FB_head->frame = NULL;
    FB_head->next = NULL;
    FB_head->prev = NULL;
    FB_head->id = 0;
    struct FrameBufferElement* prev_fbe = FB_head;
    struct FrameBufferElement* cur_frame;
    int cur_frame_id;
    for (cur_frame_id = 1; cur_frame_id < RINGBUFSIZE; cur_frame_id++) {
        cur_frame = malloc(sizeof(struct FrameBufferElement));
        cur_frame->id = cur_frame_id;
        cur_frame->frame = NULL;
        cur_frame->prev = prev_fbe;
        prev_fbe->next = cur_frame;
        prev_fbe = cur_frame;
    }
    cur_frame->next = FB_head;
    FB_head->prev = cur_frame;
    
    fd_set rfds, wfds;
    int retval;
    
    // we make the first client the client that serves our mp3 stream.
    
    listen(server, 1);
    client_len = sizeof(client_addr);
    int mp3_stream = accept(server, (struct sockaddr*) &client_addr,
                                                                 &client_len);
    
    for (;;) {
        listen(server, 10);
        // prepare fds for select
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(server, &rfds);
        FD_SET(mp3_stream, &rfds);
        struct Client* cur_client = head_client->next;
        while (cur_client != NULL) {
            FD_SET(cur_client->sock, &wfds);
            cur_client = cur_client->next;
        } 
        retval = select(max(server, mp3_stream) + 1, &rfds, NULL, NULL, NULL);
        if (retval == -1) {
            //fprintf(stderr, "select() failed, abandon process!\n");
            exit(1);
        } else if (retval) {
            client_len = sizeof(client_addr);
            if (FD_ISSET(server, &rfds)) { // add client
                client = accept(server, (struct sockaddr*) &client_addr,
                            &client_len);
                //fprintf(stderr, "Got new client!\n");
                if (client < 0) {
                    //fprintf(stderr, "Something went wrong while accepting the client!\n");
                    exit(1);
                }
                //fprintf(stderr, "allocationg new client with fd %d\n", client);
                struct Client* new_client = malloc(sizeof(struct Client));
                new_client->next = NULL;
                new_client->sent = 0;
                new_client->fbe = cur_frame;
                new_client->frame_id = cur_frame->id;
                struct Client* cur_client = head_client;
                while (cur_client->next != NULL) { // get the last client struct
                    cur_client = cur_client->next;
                }
                new_client->sock = client;
                //fprintf(stderr, "adding new client\n");
                cur_client->next = new_client;
                new_client->prev = cur_client;
                //fprintf(stderr, "added new client! %p ->* %p -> next (new client) %p\n", head_client, cur_client, new_client);
                read(client, buffer, BUFSIZE); // lawl we don't really care...
                ssize_t n = write(client, "HTTP/1.1 200 OK\r\n\r\n", 19);
                if (n < 0) {
                    remove_client(new_client);
                    //fprintf(stderr, "Life is pointless because errno: %d\n", errno);
                }
            }
            if (FD_ISSET(mp3_stream, &rfds)) { // can read mp3_stream
                if(!add_frame(mp3_stream, cur_frame)) { // frame is NULL --> EOF
                    clear_FB(cur_frame);
                    clear_clients(head_client);
                    close(mp3_stream);
                    exit(0);
                }
                cur_frame = cur_frame->next;
            }
            cur_client = head_client->next; // get the first "real" client
            while (cur_client != NULL) { // select() through our clients
                if (FD_ISSET(cur_client->sock, &wfds)) { // client can be written
                    write_data(cur_client);
                }
                cur_client = cur_client->next;
            }
            
        }
    }
/*    
    munmap(ptr, region_size);
    shm_unlink(memname);
    return 0;
*/
}
void serve_frame(int in, struct Client* clients) {
    //fprintf(stderr, "lol let's go!\n");
    struct Client* cur_client = clients;
    //fprintf(stderr, "got client pointer! %p\n", clients);
    if (clients == NULL) {
        //fprintf(stderr, "but there are no clients :(\n");
    }
    struct Frame *frame = get_frame(in);
    //fprintf(stderr, "got frame!\n");
    //print_info(frame->header);
    if (frame == NULL) {
        //fprintf(stderr, "OH NOES! frame == NULL! Assuming EOF\n");
        close(in);
        return;
    }
    //fprintf(stderr, "frame is not NULL! client fd is: %d\n", cur_client->sock);
    if (cur_client->sock == -1) { // we're the head, we don't want that
        //fprintf(stderr, "first real client is %p->%p\n", cur_client, cur_client->next);
        cur_client = cur_client->next;
    }
    while (cur_client != NULL) {
        //fprintf(stderr, "client is not NULL, but %d!\n", cur_client->sock);
        if(!write_frame(frame, cur_client)) {
            struct Client* to_remove = cur_client;
            cur_client = cur_client->next;
            remove_client(to_remove);
        } else {
            cur_client = cur_client->next;
        }
    }
    free(frame->content);
    free(frame);
}

int add_frame(int in, struct FrameBufferElement* cur_frame) {
    int id = (cur_frame->id + 1) % (RINGBUFSIZE + 1);
    cur_frame = cur_frame->next;
    if (cur_frame->frame != NULL)
        free(cur_frame->frame->content);
    free(cur_frame->frame);
    cur_frame->frame = get_frame(in);
    cur_frame->id = id;
    return cur_frame->frame != NULL;
}
    

// serve_stream() is probably unneeded
/*
void serve_stream(int in, struct Client* clients) {
    //fprintf(stderr, "lol let's go!\n");
    struct Client* cur_client = clients;
    //fprintf(stderr, "got client pointer! %p\n", clients);
    if (clients == NULL) {
        //fprintf(stderr, "but there are no clients :(\n");
    }
    struct Frame *frame = get_frame(in);
    //fprintf(stderr, "got frame!\n");
    //print_info(frame->header);
    if (frame == NULL) {
        //fprintf(stderr, "OH NOES! frame == NULL! Assuming EOF\n");
        close(in);
        return;
    }
    while (frame != NULL) {
        //fprintf(stderr, "frame is not NULL! client fd is: %d\n", cur_client->sock);
        if (cur_client->sock == -1) { // we're the head, we don't want that
            //fprintf(stderr, "first real client is %p->%p\n", cur_client, cur_client->next);
            cur_client = cur_client->next;
        }
        while (cur_client != NULL) {
            //fprintf(stderr, "client is not NULL, but %d!\n", cur_client->sock);
            if(!write_frame(frame, cur_client->sock)) {
                cur_client = cur_client->next;
                remove_client(cur_client->prev);
            } else {
                cur_client = cur_client->next;
            }
        }
        cur_client = clients;
        frame = get_frame(in);
    }
    close(in);
    return;
}
*/

void remove_client(struct Client* client) {
    //fprintf(stderr, "Dropped client with fd: %d", client->sock);
    if (client->prev != NULL) { // we're not the head
        client->prev->next = client->next;
        if (client->next != NULL) // we're not the last element
            client->next->prev = client->prev;
        if (client->sock != -1)
            close(client->sock);
        free(client);
    } else { // we're the head
        if (client->next != NULL) { // we're not alone!
            client->next = client->next->next;
            client->sock = client->next->sock;
            free(client->next);
        } else { // we're everything that's left
            if (client->sock != -1)
                close(client->sock);
            free(client);
        }
    }
}

int max(int a, int b) {
    return a > b ? a : b;
}

int write_frame(struct Frame* frame, struct Client* client) {
    size_t i;
    //fprintf(stderr, "writing frame\n");
    //fprintf(stderr, "got header 0x%08X\n", frame->header);
    //print_info(frame->header);
    uint32_t header_nl = htonl(frame->header);
    ssize_t n = 0, n_tmp = 0;
    while (n < 4) {
        n_tmp = write(client->sock, (uint8_t*) &(header_nl) + n, 4);
        n += n_tmp;
        //fprintf(stderr, "Wrote %d bytes of the header.\n", n_tmp);
        if (n_tmp < 0) {
            //fprintf(stderr, "client->sock: %d, errno: %d\n", client->sock, errno);
            ////fprintf(stderr, "EBADF: %d", EBADF);
            ////fprintf(stderr, "Something went wrong while writing %s\n", strerror(9));
            return 0;
        }
        n_tmp = 0;
    }
    //fprintf(stderr, "attempting to write frame content\n");
    n = 0;
    i = get_frame_length(frame->header) - 4;
    //fprintf(stderr, "got frame length of %d\n", i);
    while (n < i) {
        //fprintf(stderr, "before write()\n");
        n_tmp = write(client->sock, frame->content + n, i - n);
        //fprintf(stderr, "after write()\n");
        n += n_tmp;
        //fprintf(stderr, "Wrote %d bytes of the frame-content.\n", n_tmp);
        if (n_tmp < 0) {
            //fprintf(stderr, "client->sock: %d\n", client->sock);
            ////fprintf(stderr, "Something went wrong while writing %s\n", strerror(errno));
            return 0;
        }
        n_tmp = 0;
    }
    //fprintf(stderr, "Wrote %d of %d bytes.\n", n, i);
    return 1;
}

void write_data(struct Client* client) {
    if (client->fbe->frame == NULL || client->frame_id != client->fbe->id) {
    // WTF? no or wrong frame, drop the client
        remove_client(client);
    }
    size_t frame_length = get_frame_length(client->fbe->frame->header);
    ssize_t sent_tmp = 0;
    if (client->sent < 4) { // we're not done sending the header
        //fprintf(stderr, "Sent Header ");
        uint32_t header_nl = htonl(client->fbe->frame->header);
        sent_tmp = write(client->sock, &header_nl + client->sent,
                                                            4 - client->sent);
        client->sent += sent_tmp;
        //fprintf(stderr, "%d bytes\n", sent_tmp);
        sent_tmp = 0;
    }
    //fprintf(stderr, "Sent data ");
    sent_tmp = write(client->sock, client->fbe->frame->content
                            + client->sent - 4, frame_length - client->sent);
    if (sent_tmp < 0) { // write failed
        remove_client(client);
        return;
    }
    client->sent += sent_tmp;
    //fprintf(stderr, "%d bytes\n");
    if (client->sent == frame_length) {
        client->fbe = client->fbe->next;
        client->frame_id = client->fbe->id;
        client->sent = 0;
    }
    
}
    


void print_FB(struct FrameBufferElement* head)
{
    struct FrameBufferElement* cur_fbe = head->next;
    while (cur_fbe != head) {
        fprintf(stderr, "%d->", cur_fbe->id);
        cur_fbe = cur_fbe->next;
    }
    fprintf(stderr, "%d\n\n", cur_fbe->id);
    cur_fbe = head->prev;
    while (cur_fbe != head) {
        fprintf(stderr, "%d<-", cur_fbe->id);
        cur_fbe = cur_fbe->prev;
    }
    fprintf(stderr, "%d\n\n", cur_fbe->id);
}

void clear_FB(struct FrameBufferElement* elem) {
    struct FrameBufferElement* next;
    elem->prev->next = NULL;
    while (elem != NULL) {
        next = elem->next;
        if (elem->frame != NULL) {
            free(elem->frame->content);
        }
        free(elem->frame);
        free(elem);
        elem = next;
    }
}

void clear_clients(struct Client* elem) {
    struct Client* next;
    while (elem != NULL) {
        next = elem->next;
        if (elem->sock != -1)
            close(elem->sock);
        free(elem);
        elem = next;
    }
}
