#include <iostream>
#include <fstream>
#include <fileapi.h>
#include <string>
#include <dshow.h>
#pragma comment(lib, "strmiids.lib")
using namespace std;

struct Song {
    char* name;
    struct Song* next;
    struct Song* prev;
};

void add(Song **head, char* name) {
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

void remove(Song **head, Song *deleted) {
    if (*head == deleted) {
        *head = deleted->next;
    }
    if (deleted->next != NULL){
        deleted->next->prev = deleted->prev;
    }
    if (deleted->prev != NULL){
        deleted->prev->next = deleted->next;
    }
    free(deleted);
}

void instructions() {
    cout << "Type 'pause' or '||' to pause the song.\n";
    cout << "Type 'play' or '>' to play the song.\n";
    cout << "Type 'next' or '>>' to play the next song.\n";
    cout << "Type 'prev' or '<<' to play the previous song.\n";
    cout << "Type 'shuffle' to shuffle songs.\n";
    cout << "Type 'help' or 'h' to see input commands again.\n";
}

boolean play(string directory, char* name, HRESULT hr, IGraphBuilder *pGraph, IMediaControl *pControl) {
    string nameString(name);
    string fileName = directory + nameString;

    // Build the graph.
    hr = pGraph->RenderFile(wstring(fileName.begin(), fileName.end()).c_str(), NULL);

    if (SUCCEEDED(hr)) {
        // Run graph.
        hr = pControl->Run();
        return true;
    } else {
        return false;
    }

}

Song* shuffle(Song *head, int n){
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

int main() {
    HANDLE fileHandle;
    WIN32_FIND_DATAA fileData;
    ofstream file;

    IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaSeeking *pSeeking = NULL;

    struct Song* head = NULL;
    struct Song* currentSong;

    long long duration;
    long long current;
    bool validSong;
    int numberOfSongs;
    string directory;
    string fileName;
    string input;

    //finds and adds first file
    directory = "C:\\Users\\Min\\Music\\Music\\";
    fileName = directory + "*.mp3";
    fileHandle = FindFirstFileA(fileName.c_str(), &fileData);
    add(&head, fileData.cFileName);
    numberOfSongs = 1;

    //finds and adds the rest of the files
    while (FindNextFile(fileHandle, &fileData) != 0) {
        add(&head, fileData.cFileName);
        numberOfSongs++;
    }

    currentSong = head;
    instructions();

    // Initialize the COM library.
    HRESULT hr = ::CoInitialize(NULL);

    // Create the filter graph manager.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (void **)&pGraph);

    // Use returned IGraphBuilder to query.
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);

    validSong = play(directory, currentSong->name, hr, pGraph, pControl);

    // The actual music player
    while (currentSong->next != NULL) {
        cout << currentSong->name;
        cin >> input;

        // Gets duration and current position of the song.
        hr = pSeeking->GetDuration(&duration);
        hr = pSeeking->GetCurrentPosition(&current);

        // Checks to see if song is finished
        if (pSeeking->GetCurrentPosition(&current), current >= duration) {
            // Release interface pointers
            pSeeking->Release();
            pControl->Release();
            pGraph->Release();

            // Create the filter graph manager.
            hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IGraphBuilder, (void **)&pGraph);

            // Use returned IGraphBuilder to query.
            hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
            hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);

            if(!validSong) {
                while (!validSong) {
                    // Builds graph and plays song
                    validSong = play(directory, currentSong->name, hr, pGraph, pControl);
                    currentSong = currentSong->next;
                }
            }
        }

        if (input == "pause" || input == "||") {
            hr = pControl->Pause();
        } else if (input == "play" || input == ">") {
            hr = pControl->Run();
        } else if ((input == "next" || input == ">>") && currentSong->next != NULL) {
            // Sets current song to next node
            currentSong = currentSong->next;

            // Release interface pointers
            pSeeking->Release();
            pControl->Release();
            pGraph->Release();

            // Create the filter graph manager.
            hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IGraphBuilder, (void **)&pGraph);

            // Use returned IGraphBuilder to query.
            hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
            hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);

            if(!validSong) {
                while (!validSong) {
                    // Builds graph and plays song
                    validSong = play(directory, currentSong->name, hr, pGraph, pControl);
                    currentSong = currentSong->next;
                }
            }
        } else if ((input == "prev" || input == "<<") && currentSong->prev != NULL) {
            // Sets currentSong to previous node
            currentSong = currentSong->prev;

            // Release interface pointers
            pSeeking->Release();
            pControl->Release();
            pGraph->Release();

            // Create the filter graph manager.
            hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IGraphBuilder, (void **)&pGraph);

            // Use returned IGraphBuilder to query.
            hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
            hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);

                if(!validSong) {
                    while (!validSong) {
                        // Builds graph and plays song
                        validSong = play(directory, currentSong->name, hr, pGraph, pControl);
                        currentSong = currentSong->next;
                    }
                }
        } else if (input == "shuffle") {
            // Shuffles the list of songs and sets current song to head of shuffled playlist
            head = shuffle(head, numberOfSongs);
            currentSong = head;

            // Release interface pointers
            pSeeking->Release();
            pControl->Release();
            pGraph->Release();

            // Create the filter graph manager.
            hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IGraphBuilder, (void **)&pGraph);

            // Use returned IGraphBuilder to query.
            hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
            hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);

                    if(!validSong) {
                        while (!validSong) {
                            // Builds graph and plays song
                            validSong = play(directory, currentSong->name, hr, pGraph, pControl);
                            currentSong = currentSong->next;
                        }
                    }
        } else if (input == "help" || input == "h") {
            instructions();
        }
    }

    // TODO: free memory for linked list

    // Close COM library
    CoUninitialize();
    return 0;
}