#ifndef SOCKETS_SERVER_PASSIVE_SOCKET_H
#define SOCKETS_SERVER_PASSIVE_SOCKET_H

#include <stdbool.h>
#include <pthread.h>
#include "active_socket.h"

typedef struct passive_socket {
    int socket_descriptor;
    //is_listening prebieha na nom listen, čiže som zavolal accept a čakám, keď sa ku mne niekto napojí
    _Bool is_listening;
    _Bool is_waiting;
    pthread_mutex_t mutex;
    pthread_cond_t waiting_finished;
} PASSIVE_SOCKET;

void passive_socket_init(struct passive_socket* self);
void passive_socket_destroy(struct passive_socket* self);
//ak už v nejakom vlákne robím listening, tak mi to nedovolí robiť ďalší listening
//pomocou tejto metody listening za so socketu stava pasivny socket
_Bool passive_socket_start_listening(struct passive_socket* self, short port);
void passive_socket_stop_listening(struct passive_socket* self);
_Bool passive_socket_is_listening(struct passive_socket* self);
//az v tejto metode sa vola accept. Cize ked potrebujem pripojit n klientov, tak tuto metodu musim zavolat n krat
//tu este netreba volat vlakna. Staci len spravit nekonecny cyklus. Ukoncenie nekonecneho while cyklu bude podmienene ukoncenim pomocou klavesy Q
_Bool passive_socket_wait_for_client(struct passive_socket* self, struct active_socket* client_sock);

#endif //SOCKETS_SERVER_PASSIVE_SOCKET_H
