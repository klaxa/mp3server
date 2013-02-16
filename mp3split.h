#ifndef MP3SPLIT_H
#define MP3SPLIT_H


// MP3 FLAGS
#define MPEGV_MASK      0x180000
#define LAYER_MASK      0x60000
#define CRC             0x10000
#define BITRATE_MASK    0xF000
#define SAMPLING_MASK   0xC00
#define PADDING         0x200
#define PRIVATE         0x100
#define CHANNEL_MASK    0xC0
#define MODE_MASK       0x30
#define COPYRIGHT_BIT   0x8
#define ORIGINALITY_BIT 0x4
#define EMPHASIS_MASK   0x3

// MP3 CONSTANTS

// MPEG Version
#define MPEGV1          0b11
#define MPEGV2          0b10
#define MPEGV2_5        0b00

// Layer
#define LAYER1          0b11
#define LAYER2          0b10
#define LAYER3          0b01

// Channel mode
#define STEREO          0b00
#define JOINT_STEREO    0b01
#define DUAL_CHANNEL    0b10
#define SINGLE_CHANNEL  0b11

struct Frame {
    uint32_t header;
    uint8_t* content;
};

uint32_t get_first_header(int fd);
struct Frame* get_frame(int fd);
int is_header(uint32_t header);
char* int_to_binary_string(uint32_t n);
void print_info(uint32_t header);
int get_version(uint32_t header);
int get_layer(uint32_t header);
int get_crc_protection(uint32_t header);
int get_bitrate(uint32_t header);
int get_frequency(uint32_t header);
int get_padding(uint32_t header);
int get_private_bit(uint32_t header);
size_t get_frame_length(uint32_t header);
uint32_t read_header(int fd);
#endif
