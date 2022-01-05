#include <iostream>
// for reading files
#include <fileapi.h>
#include <string>
// for reading and playing music files
#include <dshow.h>
// for IGraphBuilder
#pragma comment(lib, "strmiids.lib")
using namespace std;
#include "MusicPlayer.h"

struct Song {
    char* name;
    struct Song* next;
    struct Song* prev;
};

/* Asks user for directory location, then grabs all mp3 files in that
 * location and adds their filenames to a doubly linked list.
 */
void MusicPlayer::fileHandler() {
    HANDLE fileHandle;
    WIN32_FIND_DATAA fileData;
    string fileName;

    cout << "What is the directory location of your mp3 files?";
    cin >> directory;
    directory += "\\";

    //finds and adds first file
    fileName = directory + "*.mp3";
    fileHandle = FindFirstFileA(fileName.c_str(), &fileData);
    add(&head, fileData.cFileName);
    numberOfSongs = 1;

    //finds and adds the rest of the files
    while (FindNextFile(fileHandle, &fileData) != 0) {
        add(&head, fileData.cFileName);
        numberOfSongs++;
    }
}

/* Adds one Song node to the end of a linked list */
void MusicPlayer::add(Song **head, char* name) {
    struct Song* newSong = (struct Song*)malloc(sizeof(struct Song));
    struct Song* lastSong = *head;

    char* songName = strdup(name);
    newSong->name = songName;
    newSong->next = NULL;

    //if linked list is empty, newSong becomes head
    if(*head == NULL) {
        newSong->prev = NULL;
        *head = newSong;
    } else {
        //iterates through linked list to reach the end
        while (lastSong->next != NULL) {
            lastSong = lastSong->next;
        }
        newSong->prev = lastSong;
        lastSong->next = newSong;
    }
}

/* Removes one Song node from a linked list */
void MusicPlayer::remove(Song **head, Song *deleted) {
    //if head is deleted, next Song becomes the new head
    if (*head == deleted) {
        *head = deleted->next;
    }

    // replaces prev of node after deleted node
    if (deleted->next != NULL){
        deleted->next->prev = deleted->prev;
    }

    // replaces next of node before deleted node
    if (deleted->prev != NULL){
        deleted->prev->next = deleted->next;
    }

    free(deleted);
}

/* Builds graph and plays song */
void MusicPlayer::play(string s) {
    string nameString(currentSong->name);
    string fileName = directory + nameString;

    // Create the filter graph manager.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (void **)&pGraph);

    // Use returned IGraphBuilder to query.
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pControl);
    hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **) &pSeeking);

    // Build the graph.
    hr = pGraph->RenderFile(wstring(fileName.begin(), fileName.end()).c_str(), NULL);

    // If not valid graph, move to next node in linked list.
    if (!SUCCEEDED(hr)) {
        if (s == PREV) {
            currentSong = currentSong->prev;
            release(PREV);
        } else {
            currentSong = currentSong->next;
            release(NEXT);
        }
    }

    // Gets duration and current position of the song.
    hr = pSeeking->GetDuration(&duration);
    hr = pSeeking->GetCurrentPosition(&current);

    // Run graph.
    hr = pControl->Run();
}

void MusicPlayer::pause() {
    hr = pControl->Pause();
}

void MusicPlayer::resume() {
    hr = pControl->Run();
}

void MusicPlayer::release(string s) {
    // Release interface pointers
    pSeeking->Release();
    pControl->Release();
    pGraph->Release();

    if (s != DONE) {
        play(s);
    }
}

Song* MusicPlayer::shuffle(Song *head, int n){
    //initialize random seed
    srand (time(0));
    struct Song* shuffled = NULL;
    int i = 0;
    int j;
    int random;
    int size = n;
    while (i < n) {
        struct Song *temp = head;

        //generates a random index
        random = rand() % size;
        j = 0;

        //goes to the random index
        while (j < random) {
            temp = temp->next;
            j++;
        }

        //adds the random index of the playlist to the shuffled list
        add(&shuffled, temp->name);

        //removes the random index from the playlist to prevent repeats
        remove(&head, temp);

        i++;
        size--;
    }
    return shuffled;
};

void printSongs(struct Song* song) {
    while(song != nullptr) {
        cout << song->name << "\n";
        song = song->next;
    }
}

void instructions() {
    cout << "Type 'pause' or '||' to pause the song.\n";
    cout << "Type 'play' or '>' to play the song.\n";
    cout << "Type 'next' or '>>' to play the next song.\n";
    cout << "Type 'prev' or '<<' to play the previous song.\n";
    cout << "Type 'shuffle' or 's' to shuffle playlist.\n";
    cout << "Type 'help' or 'h' to see input commands again.\n";
    cout << "Type 'exit' or 'x' to exit MusicPlayer.\n";
}

int main() {
    MusicPlayer mp;
    bool proceed = true;
    string input;

    mp.fileHandler();
    instructions();

    // Initialize the COM library.
    mp.hr = ::CoInitialize(NULL);

    mp.currentSong = mp.head;
    mp.play(mp.NEXT);

    // The actual music player
    while (proceed) {
        cout << mp.currentSong->name;
        cin >> input;

        // Checks to see if song is finished
        if (mp.current >= mp.duration) {
            mp.currentSong = mp.currentSong->next;
            mp.release(mp.NEXT);
        }

        if (input == "pause" || input == "||") {
            mp.pause();
        } else if (input == "play" || input == ">") {
            mp.resume();
        } else if ((input == "next" || input == ">>") && mp.currentSong->next != NULL) {
            // Sets current song to next node
            mp.currentSong = mp.currentSong->next;
            mp.release(mp.NEXT);
        } else if ((input == "prev" || input == "<<") && mp.currentSong->prev != NULL) {
            // Sets currentSong to previous node
            mp.currentSong = mp.currentSong->prev;
            mp.release(mp.PREV);
        } else if (input == "shuffle" || input == "s") {
            // Shuffles the list of songs and sets current song to head of shuffled playlist
            mp.head = mp.shuffle(mp.head, mp.numberOfSongs);
            mp.currentSong = mp.head;
            mp.release(mp.NEXT);
        } else if (input == "help" || input == "h") {
            instructions();
        } else if (input == "exit" || input == "x") {
            proceed = false;
        }

        if (mp.currentSong->next == NULL) {
            proceed = false;
        }
    }

    mp.release(mp.DONE);

    // Frees memory for all songs
    struct Song* temp;
    while(mp.head->next != NULL) {
        temp = mp.head;
        mp.head = mp.head->next;
        free(temp);
    }

    // Close COM library
    CoUninitialize();
    return 0;
}