// Site...: https://github.com/rafaelgm/Octavim
// Author.: Rafael Monteiro

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Stuff related to source code parsing and cell block detection
////////////////////////////////////////////////////////////////////////////////

#define IS_SPACE (*in == ' ' || *in == '\t')
#define IS_EOLN (*in == 0 || *in == '\n')
#define IS_CHAR (*in >= 'a' && *in <= 'z')
#define IS_DIGIT (*in >= '0' && *in <= '9')
#define IS_ALPHA (IS_DIGIT || IS_CHAR || *in == '_')

#define SKIP_SPACES while (IS_SPACE) in++;
#define SKIP_LINE while (!IS_EOLN) in++;

#define BEGIN         1
#define END           3
#define CELL          4
#define COMMENT_START 5
#define COMMENT_END   6

// Input string
char *in;

// Output string
static char output[10240]; // Should be more than enough!!!
char *out = output;

// Current row
int row = 1;

// Current comment nesting level
int commentNestingLevel = 0;

// Current nesting level
int nestingLevel = 0;

int hasAnyCells = 0;

// Struct to keep the events (begin, end or cell block)
struct Event
{
    int row, type, nestingLevel;
    struct Event *prev, *next;
};
struct Event *first = NULL;
struct Event *last = NULL;

void addEvent(int type)
{
    if (commentNestingLevel != 0)
        return;

    if (type == CELL)
        hasAnyCells = 1;
    else if (type == BEGIN)
        nestingLevel++;
    else if (type == END)
    {
        nestingLevel--;
        if (nestingLevel < 0)
            nestingLevel = 0;
    }

    struct Event *event = (struct Event*) malloc(sizeof(struct Event));
    event->row = type == BEGIN ? row + 1 : row;
    event->type = type;
    event->nestingLevel = nestingLevel;
    event->next = NULL;
    event->prev = last;
    if (last)
        last->next = event;
    last = event;
    if (!first)
        first = event;
}

void removeEvent(struct Event *event)
{
    if (event->next)
        event->next->prev = event->prev;
    if (event->prev)
        event->prev->next = event->next;
    if (event == first)
        first = event->next;
    if (event == last)
        last = event->prev;
    free(event);
}

void FindEvents()
{
    addEvent(BEGIN);
    last->row = 0;

    while (*in)
    {
        SKIP_SPACES;
        if (*in == '%')
        {
            // Possible comment / cell block.
            in++;
            if (*in == '%') {in++; if (IS_SPACE || IS_EOLN) addEvent(CELL); }
            if (*in == '{') {in++; if (IS_SPACE || IS_EOLN) commentNestingLevel++; }
            if (*in == '}') {in++; if (IS_SPACE || IS_EOLN) commentNestingLevel--; if (commentNestingLevel < 0) commentNestingLevel = 0; }
        }
        else if (commentNestingLevel == 0 && IS_ALPHA)
        {
            char *start = in;
            while (IS_ALPHA) in++;

            char bkp = *in;
            *in = '\0';
            if (!strcmp(start, "if")        ||
                !strcmp(start, "while")     ||
                !strcmp(start, "for")       ||
                !strcmp(start, "switch")    ||
                !strcmp(start, "try")       ||
                !strcmp(start, "function"))
            {
                addEvent(BEGIN);
            }
            else
            if (!strcmp(start, "else")      ||
                !strcmp(start, "elseif")    ||
                !strcmp(start, "case")      ||
                !strcmp(start, "otherwise") ||
                !strcmp(start, "catch"))
            {
                addEvent(END);
                addEvent(BEGIN);
            }
            else
            if (!strcmp(start, "end"))
            {
                addEvent(END);
            }

            *in = bkp;
        }
        
        // Skip the rest of the line
        SKIP_LINE;

        if (IS_EOLN)
        {
            row++;
            in++;
        }
    }
    addEvent(END);
}

int qtd = 0;
void RemoveEmptyBlocks()
{
    // Start on first->next to prevent removing the first dummy BEGIN event
    struct Event *begin = first;
    
    // Find BEGINs
    while (begin)
    {
        if (begin->type == BEGIN)
        {
            int destroy = 0;
            
            // Find it's matching END
            struct Event *end = begin->next;
            while (end)
            {
                if (end->type == CELL && end->nestingLevel == begin->nestingLevel)
                {
                    // There is a CELL within the current BEGIN. Don't remove it!
                    break;
                }
                else if (end->type == END && end->nestingLevel == begin->nestingLevel - 1)
                {
                    // We found the matching end!
                    destroy = 1;
                    break;
                }
                end = end->next;
            }
            
            // Let's destroy the pair, if necessary.
            if (destroy)
            {
                qtd++;
                struct Event *next = begin->next;
                if (begin != first)
                    removeEvent(begin);
                begin = next;

                if (end != last)
                    removeEvent(end);
                //tryAgain = 0;
                continue;
            }                
        }
        begin = begin->next;
    }
}

struct Event *findEvent(int cursorRow)
{
    struct Event *current = first;
    while (current->next)
    {
        if (current->row <= cursorRow && current->next->row > cursorRow)
            return current;
        current = current->next;
    }
    return current;
}

char * GetCell()
{
    // Get the current row (first integer on the string).
    int currentRow;
    in += sscanf(in, "%d", &currentRow);
    
    // Find events, parsing the source.
    FindEvents();
    
    if (hasAnyCells)
    {
        // Remove blocks without cell division inside
        RemoveEmptyBlocks();
        
        // Let's find which event block we are.
        struct Event *cursorEvent = findEvent(currentRow);
        
        // Find block
        int prevBlock, blockStart, blockEnd, blockStartExt, blockEndExt;

        // Start and end of the block.
        blockStart = cursorEvent->row;
        blockEnd = cursorEvent->next ? cursorEvent->next->row - 1 : blockStart;
        
        // Let's find where the block execution extends to.
        struct Event *firstEvent = cursorEvent;
        while (firstEvent->prev &&
               !(firstEvent->type == CELL &&
                 firstEvent->nestingLevel == cursorEvent->nestingLevel) &&
               firstEvent->prev->nestingLevel >= cursorEvent->nestingLevel)
        {
            firstEvent = firstEvent->prev;
        }
        blockStartExt = firstEvent->row;
        
        struct Event *lastEvent = cursorEvent;
        while (lastEvent->next &&
               !(lastEvent->next->type == CELL &&
                 lastEvent->next->nestingLevel == cursorEvent->nestingLevel) &&
               lastEvent->next->nestingLevel >= cursorEvent->nestingLevel)
        {
            lastEvent = lastEvent->next;
        }
        blockEndExt = lastEvent->next ? lastEvent->next->row : lastEvent->row;
        
        // Let's aid VIM to find where the previous block start. We don't have to do that
        // for the next block, since it starts after the current block end (not endExt).
        prevBlock = cursorEvent->prev ? cursorEvent->prev->row : 1;
        if (prevBlock == currentRow && cursorEvent->prev && cursorEvent->prev->prev)
            prevBlock = cursorEvent->prev->prev->row;
        if (prevBlock < 0)
            prevBlock = 0;

        sprintf(out, "let s:block = {'prev':%d,'start':%d,'end':%d,'startExt':%d,'endExt':%d,'z':%d}",
            prevBlock, blockStart, blockEnd, blockStartExt, blockEndExt, qtd);
    }
    else
    {
        sprintf(out, "let s:block = {}");
    }
}

////////////////////////////////////////////////////////////////////////////////
// Stuff related to send code to Octave window
////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tlhelp32.h>
#include <setupapi.h>
#include <regstr.h>

HWND getOctaveWindow(char *octaveWindowName)
{
    HWND wnd = GetTopWindow(GetDesktopWindow());
    while (wnd)
    {
        char titulo[256];
        GetWindowTextA(wnd, titulo, 255);
        if (strcmp(titulo, octaveWindowName) == 0)
             return wnd;
        wnd = GetNextWindow(wnd, GW_HWNDNEXT);
    }
    return NULL;
}

char* getClipboard()
{
    if (!OpenClipboard(NULL)) return NULL;
    char *data = NULL;
    char *handle = (char*) GetClipboardData(CF_TEXT);
    if (handle)
    {
        data = (char*) malloc(strlen(handle) + 1);
        strcpy(data, handle);
    }
    CloseClipboard();
    return data;
}

void setClipboard(char *data)
{
    if (!OpenClipboard(NULL)) return;
    
    HGLOBAL buffer;

    EmptyClipboard();
    buffer = GlobalAlloc(GMEM_DDESHARE, strlen(data) + 1);
    strcpy((char*) GlobalLock(buffer), (LPCSTR)data);
    GlobalUnlock(buffer);
    SetClipboardData(CF_TEXT, buffer);
    CloseClipboard();
}

void Run()
{
    char octaveWindowName[255];
    char *runCommand = (char*) malloc(strlen(in) + 1);

    SKIP_SPACES;
    int bytesRead;
    sscanf(in, "%[^\n]%n", octaveWindowName, &bytesRead);
    in += (bytesRead + 1);

    sscanf(in, "%[^\0]", runCommand);

    // Find Octave window
    HWND octaveWindow = getOctaveWindow(octaveWindowName);
    if (!octaveWindow) return;
    
    // Save current focused window (we will return the focus to it later)
    HWND currentWindow = GetForegroundWindow();
    
    // Bring Octave windows to Focus
    SetForegroundWindow(octaveWindow);

    // Save clipboard contents.
    char *oldClipboard = getClipboard();
    
    // Change clipboard contents to the run command call.
    setClipboard(runCommand);

    // Send CTRL+V to paste the run command and press ENTER
    keybd_event(VK_CONTROL, MapVirtualKey(VK_CONTROL, 0), 0, 0);
    keybd_event('V', MapVirtualKey('V', 0), 0, 0);
    keybd_event('V', MapVirtualKey('V', 0), KEYEVENTF_KEYUP, 0);
    //keybd_event(VK_CONTROL, MapVirtualKey(VK_CONTROL, 0), KEYEVENTF_KEYUP, 0);

    // Give Octave some time to process the request
    //Sleep(100);

    // Restore old clibboard contents.
    setClipboard(oldClipboard);

    // Let's return the focus to VIM
    SetForegroundWindow(currentWindow);
    
    free(runCommand);
}

////////////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////////////

char * OctaveHelper(char *input)
{
    // Initialize return variable.
    out[0] = 0;

    // Initialize input pointer.
    in = input;
    
    int action;
    in += sscanf(in, "%d\n", &action);

    if (action == 1)
        GetCell();
    else if (action == 2)
        Run();

    return output;
}
