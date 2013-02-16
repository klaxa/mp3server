#ifndef MP3STREAM_H
#define MP3STREAM_H

#define BUFSIZE 4096
#define RINGBUFSIZE 16

struct Client {
    int sock;
    struct Client* prev;
    struct Client* next;
    struct FrameBufferElement* fbe;
    ssize_t sent;
    unsigned int frame_id;
};

struct FrameBufferElement {
	struct Frame* frame;
	struct FrameBufferElement* prev;
	struct FrameBufferElement* next;
	unsigned int id;
};

//void serve_stream(int in, struct Client* clients);

int add_frame(int in, struct FrameBufferElement* cur_frame);
void serve_frame(int in, struct Client* clients);
void remove_client(struct Client* client);
int write_frame(struct Frame* frame, struct Client* client);
void write_data(struct Client* client);
void print_FB(struct FrameBufferElement* head);
void clear_FB(struct FrameBufferElement* head);
void clear_clients(struct Client* elem);
int max(int a, int b);
#endif
