#ifndef SOCKETS_SERVER_ACTIVE_SOCKET_H
#define SOCKETS_SERVER_ACTIVE_SOCKET_H

#include <pthread.h>
#include "char_buffer.h"
#include "linked_list.h"

GENERATE_LINKED_LIST(struct char_buffer, char_buffer);

typedef struct active_socket {
    int socket_descriptor;
    const char* end_message;
    _Bool is_reading;
    pthread_mutex_t mutex_reading;
    pthread_mutex_t mutex_writing;
    LINKED_LIST_char_buffer received_data;
    pthread_mutex_t mutex_received_data;
} ACTIVE_SOCKET;

void active_socket_init(struct active_socket* self);
void active_socket_destroy(struct active_socket* self);
//start reading je blokujuca metoda, takze do JEDNEHO VLAKNA dam vykonavanie tejto metody
//ZEVRAJ V TEJ METODE START READING JE OSETRENE TO, ZE TO NIE JE BLOKUJUCE SYSTEMOVE VOLANIE, TAKZE BY SOM MOZNO NEMUSEL ANI RIESIT VLAKNO PRE TUTO METODU
void active_socket_start_reading(struct active_socket* self);
void active_socket_stop_reading(struct active_socket* self);
_Bool active_socket_is_reading(struct active_socket* self);
//try_get_read_data si mozem zavolat v inom vlakne - dokonca by som nemusel v ziadnom vklane
//ked prisli nejake data pomocou start reading do received_data, tak si je viem vytiahnut do buffera. Ked mi metoda vrati true, tak nejake data tam uz boli. Inak false
_Bool active_socket_try_get_read_data(struct active_socket* self, struct char_buffer* output);
_Bool active_socket_is_end_message(struct active_socket* self, struct char_buffer* message);

void active_socket_write_data(struct active_socket* self, struct char_buffer* message);
void active_socket_write_end_message(struct active_socket* self);

#endif //SOCKETS_SERVER_ACTIVE_SOCKET_H
