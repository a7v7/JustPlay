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

/* Read a whole file into memory. Some virtual drives (e.g. Frame.io) break
 * PlaySound's SND_FILENAME access, but ordinary reads work everywhere. */
static void *load_file(const wchar_t *path)
{
    HANDLE h;
    LARGE_INTEGER size;
    char *buf = NULL;
    DWORD total = 0, got;

    h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return NULL;

    if (GetFileSizeEx(h, &size) && size.QuadPart > 0 && size.QuadPart < 0x7fffffff)
        buf = malloc((size_t)size.QuadPart);

    while (buf && total < (DWORD)size.QuadPart) {
        if (!ReadFile(h, buf + total, (DWORD)size.QuadPart - total, &got, NULL) || got == 0) {
            free(buf);
            buf = NULL;
        }
        total += got;
    }
    CloseHandle(h);
    return buf;
}

static int play_file(const wchar_t *path)
{
    void *data;
    BOOL ok;

    wprintf(L"Playing %ls\n", path);
    fflush(stdout);

    data = load_file(path);
    if (!data) {
        fwprintf(stderr, L"Error: cannot read %ls\n", path);
        return 0;
    }
    ok = PlaySoundW(data, NULL, SND_MEMORY | SND_SYNC | SND_NODEFAULT);
    free(data);
    if (!ok) {
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
