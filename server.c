#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#if HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <string.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "mp3split.h"
#include "mp3stream.h"
#include <errno.h>

#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

unsigned int client_count = 0;


void _usage(char *name)
{
    fprintf(stdout,"Usage: %s [options]\nOptions:\n", name);
    fprintf(stdout,"  --help\tDisplay this information\n");
#ifdef PACKAGE_NAME
    fprintf(stdout,"  --version\tDisplay %s version information\n", PACKAGE_NAME);
#endif
    fprintf(stdout,"  --v4only\tUse IPv4 sockets even if IPv6 is available\n");
    fprintf(stdout,"  --v6only\tUse IPv6 sockets even if IPv4 is available\n");
}

#define DUMMY_BUFSIZE 8


/**
 * Adds one client to the linked list
 * @param sockfd Socket of the client to add
 * @param head_client First element of the linked list
 * @param cur_frame The current element that is being streamed
 */

static inline void clientloop(int sockfd, struct Client* const head_client, struct FrameBufferElement* const cur_frame)
{
    int client = -1;
    int8_t buffer[DUMMY_BUFSIZE];
    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);

    client = accept(sockfd, (struct sockaddr*) &client_addr,
                    &client_len);

    int flags = fcntl(client, F_GETFL, 0);
    if(flags == -1)
    {
        perror(strerror(errno));
        close(client);
        return;
    }

    if(fcntl(client, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror(strerror(errno));
        close(client);
        return;
    }

    //fprintf(stderr, "Got new client!\n");
    if (client < 0) {
        //fprintf(stderr, "Something went wrong while accepting the client!\n");
        exit(1);
    }
    //fprintf(stderr, "allocationg new client with fd %d\n", client);
    struct Client* new_client = malloc(sizeof(struct Client));
    if(!new_client) {
        perror(strerror(errno));
        exit(1);
    }
    new_client->next = NULL;
    new_client->sent = 0;
    new_client->fbe = cur_frame;
    new_client->frame_id = cur_frame->id;
    new_client->skipped_frames = 0;
    struct Client* cur_client = head_client;
    while (cur_client->next != NULL) { // get the last client struct
        cur_client = cur_client->next;
    }
    new_client->sock = client;
    //fprintf(stderr, "adding new client\n");
    cur_client->next = new_client;
    new_client->prev = cur_client;
    //fprintf(stderr, "added new client! %p ->* %p -> next (new client) %p\n", head_client, cur_client, new_client);
    socklen_t addr_len = sizeof(client_addr);
    int err = getnameinfo((struct sockaddr*)&client_addr,addr_len,new_client->ip,sizeof(new_client->ip),
    0,0,NI_NUMERICHOST);
    fprintf(stderr, "Client %d (%s) connected, clients: %d\n",
            new_client->sock,new_client->ip , ++client_count);
    read(client, buffer, DUMMY_BUFSIZE); // lawl we don't really care...
    ssize_t n = write(client,
                      "HTTP/1.1 200 OK\r\n\r\n", 19);
                      //icy-metaint: 8192\r\n\r\n", 38);
    if (n < 0) {
        remove_client(new_client);
        //fprintf(stderr, "Life is pointless because errno: %d\n", errno);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 1) {
        //fprintf(stderr, "NO U >:V giev host and port\n");
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);
    
    struct Client* head_client = malloc(sizeof(struct Client));
    if(!head_client) {
        perror(strerror(errno));
        exit(1);
    }
    
    head_client->sock = -1;
    head_client->prev = NULL;
    head_client->next = NULL; 
    
    int server4 = -1, server6 = -1, v4only = 0, v6only = 0, opts;
    unsigned int port = 8080;
    socklen_t client_len;
    struct sockaddr_storage client_addr;
    struct addrinfo hints, *res;
    char port_str[5];
    int c, error = 0, option_index = 0;
#ifdef HAVE_GETOPT_LONG
    struct option long_options[] = {
        {"v4only", no_argument, &v4only, 1},
        {"v6only", no_argument, &v6only, 1},
        {"version", no_argument, NULL, 'V'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };
#endif

    while(1) {
#ifdef HAVE_GETOPT_LONG
        c = getopt_long(argc, argv, "46Vh",
                long_options, &option_index);
#else
	c = getopt(argc, argv, "46Vh");
#endif
        if(c == -1)
            break;

        switch(c) {
             case '4':
                if(v6only) {
                    fprintf(stderr, "%s: cannot specify -4 and -6 together\n", argv[0]);
                    exit(1);
                }
                 v4only = 1;
                 break;
            case '6':
                if(v4only) {
                    fprintf(stderr, "%s: cannot specify -6 and -4 together\n", argv[0]);
                    exit(1);
                }
                v6only = 1;
                break;
            case 'V':
#ifdef PACKAGE_STRING
                fprintf(stdout, "%s\n", PACKAGE_STRING);
#endif
                exit(0);
                break;
            case 'h':
#ifdef PACKAGE_STRING
                fprintf(stdout, "%s\n", PACKAGE_STRING);
#endif
                _usage(argv[0]);
                exit(0);
                break;
            case '?':
                fprintf(stderr,
                "Unrecognized option: -%c\n", optopt);
                exit(1);
                break;
        }
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(port_str, sizeof(port_str), "%d", port);

    error = getaddrinfo(NULL, port_str, &hints, &res);
    if (error) {
        perror(gai_strerror(error));
        exit(1);
    }

    if (!v4only && (server6 = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        if(errno != EAFNOSUPPORT
#ifdef EPFNOSUPPORT
	 && errno != EPFNOSUPPORT
#endif
		) {
            perror(strerror(errno));
            exit(1);
        }
        else
            v4only = 1;
    }

    if(!v6only && (server4 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        if(errno != EAFNOSUPPORT
#ifdef EPFNOSUPPORT
	 && errno != EPFNOSUPPORT
#endif
	) {
            perror(strerror(errno));
            exit(1);
        }
        else
            v6only = 1;
    }

    int on = 1;
    if (server4 > 0 && setsockopt(server4, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
        //fprintf(stderr, "Something went wrong while setting up the server!\n");
        perror(strerror(errno));
        exit(1);
    }

    if (server6 > 0 && setsockopt(server6, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
        //fprintf(stderr, "Something went wrong while setting up the server!\n");
        perror(strerror(errno));
        exit(1);
    }

#ifdef IPV6_V6ONLY
    /* on supported operating systems, make the IPv6 socket listen only for IPv6 connections */
    if (server6 > 0 && setsockopt(server6, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) != 0) {
        perror(strerror(errno));
        exit(1);
    }
#endif

    /* make IPv4 listening socket non-blocking 
    if(server4 > 0) {
        if((opts = fcntl(server4, F_GETFL)) == -1)
        {
            perror(strerror(errno));
            exit(1);
        }

        opts = (opts | O_NONBLOCK);
        if (fcntl(server4, F_SETFL, opts) < 0)
        {
            perror(strerror(errno));
            exit(1);
        }
    }
    */
    /* make IPv6 listening socket non-blocking 
    if(server6 > 0) {
        if((opts = fcntl(server6, F_GETFL)) == -1)
        {
            perror(strerror(errno));
            exit(1);
        }

        opts = (opts | O_NONBLOCK);
        if (fcntl(server6, F_SETFL, opts) < 0)
        {
            perror(strerror(errno));
            exit(1);
        }
    }
    */
    for(struct addrinfo *iter = res; iter; iter = iter->ai_next) {
        int server = (iter->ai_family == PF_INET6 ? server6 : server4);
        if (server > 0 && bind(server, iter->ai_addr,
                 iter->ai_addrlen) < 0) {
                close(server);
                server = -1;
                fprintf(stderr, "Something went wrong while binding the server! %s\n",
                        strerror(errno));
        }
    }
    freeaddrinfo(res);
    res = NULL;
    
    struct FrameBufferElement* FB_head = malloc(sizeof(
                                          struct FrameBufferElement));
    if(!FB_head) {
        perror(strerror(errno));
        exit(1);
    }

    FB_head->frame = NULL;
    FB_head->next = NULL;
    FB_head->prev = NULL;
    FB_head->id = 0;
    struct FrameBufferElement* prev_fbe = FB_head;
    struct FrameBufferElement* cur_frame;
    int cur_frame_id;
    for (cur_frame_id = 1; cur_frame_id < RINGBUFSIZE; cur_frame_id++) {
        cur_frame = malloc(sizeof(struct FrameBufferElement));
        if(!cur_frame) {
            perror(strerror(errno));
            exit(1);
        }
        cur_frame->id = cur_frame_id;
        cur_frame->frame = NULL;
        cur_frame->prev = prev_fbe;
        cur_frame->latest = 0;
        prev_fbe->next = cur_frame;
        prev_fbe = cur_frame;
    }
    cur_frame->next = FB_head;
    FB_head->prev = cur_frame;
    
    fd_set master_fds, rfds, wfds;
    int retval;
    
    FD_ZERO(&master_fds);

    // we make the first client the client that serves our mp3 stream.
    
    if (server4 > 0) { listen(server4, 1); FD_SET(server4, &master_fds); }
    if (server6 > 0) { listen(server6, 1); FD_SET(server6, &master_fds); }

    //memcpy(&rfds, &master_fds, sizeof(master_fds));
    rfds = master_fds;
    
    retval = select(max(server4, server6) + 1, &rfds, NULL, NULL, NULL);
    if (retval == -1) exit(1);

    client_len = sizeof(client_addr);
    
    int mp3_stream = -1;

    if (FD_ISSET(server4, &rfds)) {
        mp3_stream = accept(server4, (struct sockaddr*) &client_addr,
                            &client_len);
    } else if (FD_ISSET(server6, &rfds)) {
        mp3_stream = accept(server6, (struct sockaddr*) &client_addr,
                            &client_len);
    }

    unsigned int frame_number = 0;
    for (;;) {
        // prepare fds for select
        memcpy(&rfds, &master_fds, sizeof(master_fds));
        FD_ZERO(&wfds);
        if (server4 > 0) listen(server4, 10);
        if (server6 > 0) listen(server6, 10);
        FD_SET(mp3_stream, &rfds);
        struct Client* cur_client = head_client->next;
        while (cur_client != NULL) {
            FD_SET(cur_client->sock, &wfds);
            cur_client = cur_client->next;
        }
        retval = select(max(max(server4, server6), mp3_stream) + 1, &rfds, NULL, NULL, NULL);
        if (retval <= 0) {
            //fprintf(stderr, "select() failed, abandon process!\n");
            exit(1);
        } else if (retval) {
            if (FD_ISSET(server6, &rfds)) clientloop(server6, head_client, cur_frame);
            if (FD_ISSET(server4, &rfds)) clientloop(server4, head_client, cur_frame);

            if (FD_ISSET(mp3_stream, &rfds)) { // can read mp3_stream
                if(!add_frame(mp3_stream, cur_frame)) { // frame is NULL --> EOF
                    clear_FB(cur_frame);
                    clear_clients(head_client);
                    close(mp3_stream);
                    exit(0);
                }
                cur_frame = cur_frame->next;
                //fprintf(stderr, "Frame: %d, Buffer: %d\n", frame_number,
                //                                                cur_frame->id);
                frame_number++;
            }
            cur_client = head_client->next; // get the first "real" client
            while (cur_client != NULL) { // select() through our clients
                if (FD_ISSET(cur_client->sock, &wfds)) { // client can be written
                //    fprintf(stderr, "Client: %d, Frame: %d\n", cur_client->sock,
                //        cur_client->frame_id);
                    write_data(cur_client);
                }
                cur_client = cur_client->next;
            }
        }
    }
}
/**
 * Adds one frame from the stream to the buffer and puts it behind the currently
 * streamed frame in the buffer.
 * @param in The stream we are taking our frame from
 * @param cur_frame The current frame that is being streamed
 * @return Returns whether or not the frame we got is valid (1) or not (0)
 */
int add_frame(int in, struct FrameBufferElement* cur_frame) {
    int id = (cur_frame->id + 1) % (RINGBUFSIZE + 1);
    cur_frame->latest = 0;
    cur_frame = cur_frame->next;
    if (cur_frame->frame != NULL)
        free(cur_frame->frame->content);
    free(cur_frame->frame);
    cur_frame->frame = get_frame(in);
    cur_frame->id = id;
    cur_frame->latest = 1;
    return cur_frame->frame != NULL;
}

/**
 * Removes one client from the client list
 * @param client The client to be removed.
 */
void remove_client(struct Client* client) {
    fprintf(stderr, "Client %d (%s) dropped, clients: %d\n", client->sock,
    client->ip, --client_count);
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
/**
 * Returns the maximum of two given integers
 * @param a The first value
 * @param b The second value
 * @return The greater value
 */
int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * Writes data to one client, this is not trivial.
 * The function checks if there is data to be sent to the client, by checking
 * if the current frame pointed to by the client is the most recent one. If not
 * one frame is sent to the client.
 * @param client The client we are sending data to.
 */
void write_data(struct Client* client) {
    if (client->fbe->frame == NULL) {
        // WTF? no frame, drop the client
        remove_client(client);
    }
    if (client->frame_id != client->fbe->id) {
        // client will skip, lagged behind too long in the ringbuffer
        fprintf(stderr, "Client %d lagging behind %d frames.\n", client->sock,
                                                        client->skipped_frames);
        if (client->skipped_frames++ > RINGBUFSIZE * 2) {
            // assume the client has quit
            remove_client(client);
            return;
        }
    } else {
        client->skipped_frames = 0;
    }
    size_t frame_length;
    ssize_t sent_tmp;
    uint32_t header_nl;
    while (!client->fbe->latest) {
        frame_length = get_frame_length(client->fbe->frame->header);
        sent_tmp = 0;
        if (client->sent < 4) { // we're not done sending the header
            //fprintf(stderr, "Sent Header ");
            header_nl = htonl(client->fbe->frame->header);
            sent_tmp = write(client->sock, &header_nl + client->sent,
                                                                4 - client->sent);
            client->sent += sent_tmp;
            //fprintf(stderr, "%d bytes\n", sent_tmp);
            sent_tmp = 0;
        }
        //fprintf(stderr, "Sent data ");
        sent_tmp = write(client->sock, client->fbe->frame->content
                                + client->sent - 4, frame_length - client->sent);
        if (sent_tmp < 0) { // write failed, check for error
            if (errno == EPIPE) // client quit
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
}
/**
 * Prints pointers of all framebuffer elements
 * @param head The head of the list, actually any element from the list, since
 * it is a ringbuffer list
 */
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
/**
 * Cleans up the Ringbuffer
 * @param elem Any element from the ringbuffer
 */

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
/**
 * Cleans up the client list
 * @param elem The head client
 */
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
