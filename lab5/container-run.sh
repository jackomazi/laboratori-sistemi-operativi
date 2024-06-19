#!/bin/bash

# Verifica del numero di argomenti
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 conf-file command [args...]"
    exit 1
fi

# Lettura degli argomenti
CONF_FILE="$1"
COMMAND="$2"
shift 2
COMMAND_ARGS="$@"

# Creazione di una directory temporanea
WORKDIR=$(mktemp -d)

# Funzione per pulire al termine dello script
cleanup() {
    echo "Cleaning up..."
    umount -l "$WORKDIR"/* > /dev/null 2>&1
    rm -rf "$WORKDIR"
}
trap cleanup EXIT

# Lettura del file di configurazione
IFS=$'\n'
for line in $(cat "$CONF_FILE"); do
    ORIGIN=$(echo "$line" | awk '{print $1}')
    DEST=$(echo "$line" | awk '{print $2}')
    
    # Creazione delle directory necessarie
    DEST_DIR=$(dirname "$WORKDIR/$DEST")
    mkdir -p "$DEST_DIR"
    
    # Gestione di file e directory
    if [ -d "$ORIGIN" ]; then
        bindfs --no-allow-other "$ORIGIN" "$WORKDIR/$DEST"
    elif [ -f "$ORIGIN" ]; then
        cp "$ORIGIN" "$WORKDIR/$DEST"
    else
        echo "Error: $ORIGIN is not a valid file or directory"
        exit 2
    fi
done

# Esecuzione del comando nel container
fakechroot fakeroot chroot "$WORKDIR" "$COMMAND" $COMMAND_ARGS
