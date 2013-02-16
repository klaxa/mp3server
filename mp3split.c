#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "mp3split.h"


/*int main(int argc, char **argv ) {
    if (argc < 2) {
        //fprintf(stderr, "No fuck you, give me a file to work with!\n");
        exit(1);
    }
    FILE *fp;
    uint32_t header;
    fp = fopen(argv[1], "r");
    header = get_first_header(fp);
    print_info(header);
    size_t frame_length = get_frame_length(header);
    uint8_t *frame = malloc(frame_length);
    
    //fprintf(stderr, "Read %u bytes of a frame and meant to read %u.\n", fread(frame, sizeof(uint8_t), frame_length, fp), frame_length);
    int new_frame_length;
    while (header = read_header(fp)) {
        //fprintf(stderr, "Header: %s (0x%08X) offset: 0x%08X\n", int_to_binary_string(header),
        header, ftell(fp) - 4);
        print_info(header);
        new_frame_length = get_frame_length(header);
        if (new_frame_length != frame_length && new_frame_length != -1) {
            free(frame);
            uint8_t *frame = malloc(new_frame_length);
            frame_length = new_frame_length;
        }
        //fprintf(stderr, "Read %u bytes of a frame and meant to read %u.\n", fread(frame, sizeof(uint8_t), frame_length, fp));
        fwrite(frame, sizeof(uint8_t), frame_length, stdout);
    }
    fclose(fp);
    return 0;
}
*/
struct Frame* get_frame(int fd) {
    uint32_t header;
    //if (first_read) {
        //fprintf(stderr, "Getting first header\n");
        header = get_first_header(fd);
        if (header == -1) {
            return NULL;
        }
    //    first_read = 0;
    //} else {
    //    //fprintf(stderr, "Getting next header\n");
    //    header = read_header(fd);
    //}
    print_info(header);
    size_t frame_length = get_frame_length(header);
    struct Frame* frame = malloc(sizeof(struct Frame));
    frame->content = malloc(frame_length);
    frame->header = header;
    ssize_t n = 0, n_tmp = 0;
    //fprintf(stderr, "Attempting to read %d bytes.\n", frame_length - 4);
    while (n < frame_length - 4) {
        n_tmp = read(fd, frame->content + n, frame_length - n - 4);
        //fprintf(stderr, "Read %d bytes of %d.\n", n_tmp, frame_length - n - 4);
        if (n_tmp == 0) {
            //fprintf(stderr, "Reached EOF\n");
            free(frame->content);
            free(frame);
            return NULL;
        }
        if (n_tmp < 0) {
            //fprintf(stderr, "Error %s", strerror(errno));
            free(frame->content);
            free(frame);
            return NULL;
        }
        n += n_tmp;
        n_tmp = 0;
    }
    //fprintf(stderr, "frame: %p\n", frame);
    return frame;
}

uint32_t get_first_header(int fd) {
    uint8_t cur_byte;
    uint32_t header = 0;
    ssize_t n = 0;
    while (n < 1) {
        //fprintf(stderr, "fd: %d\n", fd);
        n = read(fd, &cur_byte, 1);
        //fprintf(stderr, "Read %d bytes\n", n);
        if (n < 0) {
            //fprintf(stderr, "don't die now pls\n");
            ////fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(1);
        }
        if (n == 0) {
            //fprintf(stderr, "EOF, lol\n");
            return -1;
        }
        header = ((header << 8) | cur_byte);
        if(is_header(header)) {
            //fprintf(stderr, "found header\n");
            return header;
        }
        n = 0;
    }
    close(fd);
    exit(1);
    return -1; // we should never reach this point in the first place...
}

int is_header(uint32_t header) {
    //if (header >= 0xFFE00000) {
        ////fprintf(stderr, "Header: %s (0x%08X) offset: 0x%08X\n", int_to_binary_string(header),
        //header, ftell(fp));
        ////fprintf(stderr, " X\n");
        //print_info(header);
        //return 1;
        ////fprintf(stderr, "Frame lentgh:\t%d\n", get_frame_length(header));
    /* else {
        //fprintf(stderr, "\n");
    }*/
    //return 0;
    
    return (header >= 0xFFE00000 && get_version(header) > 0 
        && get_layer(header) > 0 && get_bitrate(header) > 0
        && get_frequency(header) > 0);
}

char* int_to_binary_string(uint32_t n) {
    char* str = malloc(sizeof(char) * 32);
    
    unsigned int i;
    for (i = 0; i < 32; i++) {
        *(str + i) = (n>>31) + '0';
        n = n<<1;
    }
    return str;
}

void print_info(uint32_t header) {
    //fprintf(stderr, "Hex repres.   %08X\n", header);
    //fprintf(stderr, "MPEG Version: %d\n", get_version(header));
    //fprintf(stderr, "Layer:        %d\n", get_layer(header));
    //fprintf(stderr, "CRC:          %s\n", get_crc_protection(header) ? "yes" : "no");
    //fprintf(stderr, "Bitrate:      %d kbps\n", get_bitrate(header));
    //fprintf(stderr, "Frequency:    %d Hz\n", get_frequency(header)); 
    //fprintf(stderr, "Padding:      %s\n", get_padding(header) ? "yes" : "no");
    //fprintf(stderr, "Private bit:  %s\n", get_private_bit(header) ? "yes" : "no");
    
    //fprintf(stderr, "Channels:     %s\n", get_channel_mode(header) == STEREO ? "Stereo" :
//                     get_channel_mode(header) == JOINT_STEREO ? "Joint Stereo" :
//                     get_channel_mode(header) == DUAL_CHANNEL ? "Dual Channel" :
//                             "Single channel");
    //fprintf(stderr, "Frame length: %u\n", get_frame_length(header));
    // We don't really care about the rest
    /*
    if (get_channel_mode(header) == JOINT_STEREO) {
        //fprintf(stderr, "Mode:         %u\n", get_mode_extension(header));
    }
    
    //fprintf(stderr, "Copyrighted:  %s\n", get_copyright(header) ? "yes" : "no");
    //fprintf(stderr, "Original:     %s\n", get_originality(header) ? "yes" : "no");
    //fprintf(stderr, "Emphasis:     %u\n", get_emphasis(header)); */
}

int get_version(uint32_t header) {
    switch((header & MPEGV_MASK) >> 19) {
        case MPEGV1:
            return 1;
        case MPEGV2:
            return 2;
        case MPEGV2_5:
            return 3;
        default:
            return -1;
    }
}

int get_layer(uint32_t header) {
    switch((header & LAYER_MASK) >> 17) {
        case LAYER1:
            return 1;
        case LAYER2:
            return 2;
        case LAYER3:
            return 3;
        default:
            return -1;
    }
}

int get_crc_protection(uint32_t header) {
    return (int) (header & CRC) >> 16;
}

int get_bitrate(uint32_t header) {
    int version = get_version(header);
    int layer = get_layer(header);
    if (version == -1 || layer == -1) {
        return -1;
    }
    
    if (version == 3) {
        version = 2;
    }
    
    int bitrate = (int) (header & BITRATE_MASK) >> 12;
    
    if (bitrate == 0) {
        return 0;
    }
    
    if (version == 1 && layer == 1) {
        if (bitrate < 15) {
            return 32 * bitrate;
        } else {
            return -1;
        }
    }
    
    if (version == 2 && (layer == 2 || layer == 3)) {
        if (bitrate <= 8) {
            return bitrate * 8;
        } else if (bitrate < 15 ) {
            return 64 + (bitrate & 7) * 16;
        } else {
            return -1;
        }
    }
    
    if ((version == 1 && layer == 2) || (version == 2 && layer == 1)) {
        if (bitrate <= 2) {
            return (bitrate + 1) * 16;
        } else if (bitrate == 3) {
            return 56;
        } else if (bitrate <= 8 ) {
            return (bitrate - 4) * 16 + 64;
        } else if (bitrate  <= 12) {
            return 128 + (bitrate - 8) * layer * 16;
        } else if (bitrate <= 14) {
            return 128 + layer * 64 + (bitrate - 12) * layer * 32;
        } else {
            return -1;
        }
    }
    
    if ((version == 1 && layer == 3)) {
        if (bitrate <= 5) {
            return 32 + (bitrate - 1) * 8;
        } else if (bitrate <= 9) {
            return 16 * (bitrate - 1);
        } else if (bitrate <= 13) {
            return 128 + (bitrate - 9) * 32;
        } else if (bitrate == 14) {
            return 320;
        } else {
            return -1;
        }
    }
}


int get_frequency(uint32_t header) {
    int version = get_version(header);
    int freq = (header & SAMPLING_MASK) >> 10;
    if (freq >= 3) {
        return -1;
    }
    if (version != 1) {
    
        if (freq == 0) {
            return 11025 * (3 - version);
        } else if (freq == 1) {
            return 12000 * (3 - version);
        } else if (freq == 2) {
            return 8000 * (3 - version);
        } else {
            return -1;
        }
    } else {
        if (freq == 0) {
            return 44100;
        } else if (freq == 1) {
            return 48000;
        } else if (freq == 2) {
            return 32000;
        } else {
            return -1;
        }
    }
}

int get_padding(uint32_t header) {
    return (header & PADDING) >> 9;
}

int get_private_bit(uint32_t header) {
    return (header & PRIVATE) >> 8;
}
int get_channel_mode(uint32_t header) {
    return (header & CHANNEL_MASK) >> 6;
}
// frame_length excludes header
size_t get_frame_length(uint32_t header) {
    int version = get_version(header);
    int layer = get_layer(header);
    int bitrate = get_bitrate(header);
    int freq = get_frequency(header);
    if (layer == 1) {
        return (size_t) (12 * bitrate * 1000 / freq + get_padding(header)) * 4;
    } else if (layer == 2 || layer == 3) {
        return (size_t) 144 * bitrate * 1000 / freq + get_padding(header);
    } else {
        return (size_t) -1;
    }
}

uint32_t read_header(int fd) {
    uint8_t cur_byte;
    uint32_t header;
    int i;
    for (i = 0; i < 4; i++) {
        read(fd, &cur_byte, 1);
        header = ((header << 8) | cur_byte);
    }
    return header;
}
