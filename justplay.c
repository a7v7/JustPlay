/* JustPlay - a minimal WAV player for Windows.
 *
 * Usage: justplay <file.wav | directory>
 */
#include <windows.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define JUSTPLAY_VERSION L"1.0.0"

static void print_help(void)
{
    wprintf(L"JustPlay %ls - minimal WAV player\n"
            L"\n"
            L"Usage: justplay <file.wav | directory>\n"
            L"  file.wav   play the file on the default output device\n"
            L"  directory  play all .wav files in it, in Explorer sort order\n"
            L"\n"
            L"Paths longer than 260 characters are not supported.\n",
            JUSTPLAY_VERSION);
}

static int play_file(const wchar_t *path)
{
    wprintf(L"Playing %ls\n", path);
    fflush(stdout);
    if (!PlaySoundW(path, NULL, SND_FILENAME | SND_SYNC | SND_NODEFAULT)) {
        fwprintf(stderr, L"Error: cannot play %ls\n", path);
        return 0;
    }
    return 1;
}

static int compare_names(const void *a, const void *b)
{
    return StrCmpLogicalW(*(const wchar_t *const *)a, *(const wchar_t *const *)b);
}

static int play_directory(const wchar_t *dir)
{
    wchar_t pattern[MAX_PATH];
    WIN32_FIND_DATAW fd;
    HANDLE h;
    wchar_t **names = NULL;
    size_t count = 0, cap = 0, i;
    int failures = 0;

    swprintf(pattern, MAX_PATH, L"%ls\\*.wav", dir);
    h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        fwprintf(stderr, L"No .wav files found in %ls\n", dir);
        return 1;
    }

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        if (count == cap) {
            cap = cap ? cap * 2 : 32;
            names = realloc(names, cap * sizeof *names);
            if (!names) {
                fwprintf(stderr, L"Out of memory\n");
                FindClose(h);
                return 1;
            }
        }
        names[count++] = _wcsdup(fd.cFileName);
    } while (FindNextFileW(h, &fd));
    FindClose(h);

    qsort(names, count, sizeof *names, compare_names);

    for (i = 0; i < count; i++) {
        wchar_t path[MAX_PATH];
        swprintf(path, MAX_PATH, L"%ls\\%ls", dir, names[i]);
        if (!play_file(path))
            failures++;
        free(names[i]);
    }
    free(names);
    return failures ? 1 : 0;
}

int wmain(int argc, wchar_t **argv)
{
    DWORD attr;

    if (argc != 2) {
        print_help();
        return argc == 1 ? 0 : 1;
    }

    attr = GetFileAttributesW(argv[1]);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        fwprintf(stderr, L"Error: %ls not found\n", argv[1]);
        return 1;
    }

    if (attr & FILE_ATTRIBUTE_DIRECTORY)
        return play_directory(argv[1]);
    return play_file(argv[1]) ? 0 : 1;
}
