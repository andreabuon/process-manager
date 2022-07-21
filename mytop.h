#pragma once
#ifndef MYTOP_H
#define MYTOP_H

#include <sys/types.h>

//Visualizza una finestra con il messaggio di errore passato.
void showErrorDialog(char* error);

//Invia il segnale signal_n al processo selezionato nella TreeView. Infine aggiorna i dati della riga corrispondente.
void sendSignal(int signal_n);

#endif //MYTOP_H