#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CHUNK_SIZE 1024 * 1024 * 1024 // 1GB chunk size

int main() {
    // Open the MIDI file for writing
    FILE* midiFile = fopen("output.mid", "wb");

    // Set the MIDI file's header chunk
    putc('M', midiFile);
    putc('T', midiFile);
    putc('h', midiFile);
    putc(0x00, midiFile); // Chunk size
    putc(0x00, midiFile);
    putc(0x00, midiFile);
    putc(0x06, midiFile); // MIDI format type 1
    putc(0x00, midiFile); // Number of tracks
    putc(0x01, midiFile); // Number of tracks
    putc(0x00, midiFile); // Time division (ticks per quarter note)
    putc(0x80, midiFile); // Time division (ticks per quarter note)

    // Set the MIDI file's first track chunk
    fputs("MTrk", midiFile); // Track ID
    int trackChunkSize = 0; // Track chunk size
    putc((trackChunkSize >> 24) & 0xFF, midiFile);
    putc((trackChunkSize >> 16) & 0xFF, midiFile);
    putc((trackChunkSize >> 8) & 0xFF, midiFile);
    putc(trackChunkSize & 0xFF, midiFile);

    // Generate random MIDI events and append them to the file in chunks
    srand(time(NULL));
    while (ftell(midiFile) < 6LL * 1024 * 1024 * 1024) { // check file size
        // Write the MIDI events to the file in chunks
        int bytesWritten = 0;
        while (bytesWritten < CHUNK_SIZE) {
            // Generate a random MIDI event
            int deltaTime = rand() % 10000; // random delta time up to 10000 ticks
            int status = rand() % 256; // random status byte
            int data1 = rand() % 128; // random data byte 1
            int data2 = rand() % 128; // random data byte 2
            int eventLength = 0;
            if ((status >= 0x80 && status <= 0xEF) || status == 0xF1 || status == 0xF2 || status == 0xF3) {
                // MIDI events with two data bytes
                eventLength = 3;
            } else if (status == 0xF0 || status == 0xF7) {
                // System Exclusive events with variable length data
                eventLength = 2 + (rand() % 1024);
            } else {
                // MIDI events with one data byte or meta events
                eventLength = 2;
            }

            // Append the MIDI event to the file
            putc(deltaTime & 0x7F, midiFile); // Delta time (LSB)
            while (deltaTime >= 0x80) {
                deltaTime >>= 7;
                putc((deltaTime & 0x7F) | 0x80, midiFile); // Delta time continuation (MSB)
            }
             putc(status, midiFile);
            for (int i = 0; i < eventLength; i++) {
                putc(rand() % 256, midiFile); // Data byte
            }

            bytesWritten = ftell(midiFile);
            if (bytesWritten >= 6LL * 1024 * 1024 * 1024) { // check file size
                break;
            }
        }

        // Update the track chunk size and append another track chunk if needed
        trackChunkSize += bytesWritten;
        if (trackChunkSize >= CHUNK_SIZE) {
            fputs("MTrk", midiFile); // Track ID
            trackChunkSize = 0;
            putc((trackChunkSize >> 24) & 0xFF, midiFile);
            putc((trackChunkSize >> 16) & 0xFF, midiFile);
            putc((trackChunkSize >> 8) & 0xFF, midiFile);
            putc(trackChunkSize & 0xFF, midiFile);
        }
    }

    // Set the track chunk size
    fseek(midiFile, 18, SEEK_SET);
    putc((trackChunkSize >> 24) & 0xFF, midiFile); // write the track chunk size
    putc((trackChunkSize >> 16) & 0xFF, midiFile);
    putc((trackChunkSize >> 8) & 0xFF, midiFile);
    putc(trackChunkSize & 0xFF, midiFile);

    // Close the MIDI file
    fclose(midiFile);

    return 0;
}