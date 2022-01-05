#ifndef MUSICPLAYER_MUSICPLAYER_H
#define MUSICPLAYER_MUSICPLAYER_H


class MusicPlayer {
private:
    IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaSeeking *pSeeking = NULL;

public:
    struct Song* head = NULL;
    struct Song* currentSong;

    HRESULT hr;
    int numberOfSongs;
    long long duration;
    long long current;
    string directory;

    const string PREV = "prev";
    const string NEXT = "next";
    const string DONE = "done";

    void fileHandler();
    void add(Song **head, char* name);
    void remove(Song **head, Song *deleted);
    void play(string s);
    void release(string s);
    void resume();
    void pause();
    Song* shuffle(Song *head, int n);
};


#endif //MUSICPLAYER_MUSICPLAYER_H
