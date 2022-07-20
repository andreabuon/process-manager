#pragma once
#ifndef MYTOP_H
#define MYTOP_H

#include <sys/types.h>

//Visualizza una finestra con il messaggio di errore passato.
void showErrorDialog(char* error);

//Ritorna il pid del processo selezionato nella TreeView. Ritorna -1 se nessuna riga è stata selezionata.
pid_t getSelectedPID();

//Aggiorna i dati del processo selezionato nella TreeView.
void updateRow();

#endif //MYTOP_H