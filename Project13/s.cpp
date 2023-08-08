#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cctype>
#include <algorithm>

// ----- WordleGame Class -----
class WordleGame {
private:
    std::vector<std::string> words;
    std::string targetWord;
    int attempts;

    std::vector<std::string> loadWords(const std::string& filename) {
        std::ifstream infile(filename);
        std::vector<std::string> loadedWords;
        std::string word;

        while (infile >> word) {
            loadedWords.push_back(word);
        }

        return loadedWords;
    }

    std::string selectRandomWord() {
        return words[rand() % words.size()];
    }

public:
    WordleGame(const std::string& filename) {
        srand(static_cast<unsigned int>(time(0)));
        words = loadWords(filename);
        targetWord = selectRandomWord();
        attempts = 0;
    }

    std::vector<std::string> guess(const std::string& word) {
        attempts++;
        std::vector<std::string> feedback;

        for (size_t i = 0; i < word.size(); ++i) {
            if (word[i] == targetWord[i]) {
                feedback.push_back("green");
            }
            else if (targetWord.find(word[i]) != std::string::npos) {
                feedback.push_back("yellow");
            }
            else {
                feedback.push_back("gray");
            }
        }

        return feedback;
    }

    bool isValidWord(const std::string& word) const {
        return std::find(words.begin(), words.end(), word) != words.end();
    }

    std::string getTargetWord() const {
        return targetWord;
    }
};

// Structure for Guess and Feedback
struct GuessFeedback {
    std::string guess;
    std::vector<std::string> feedback;
};

// ----- Windows Application Code -----
WordleGame game("D://sortedwords.txt");
std::vector<GuessFeedback> pastGuesses = { {"", {}} };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_CREATE:
    {
        CreateWindowA(
            "BUTTON",
            "Submit",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            320, 10, 80, 30,
            hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        CreateWindowA(
            "BUTTON",
            "Show Answer",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            420, 10, 100, 30,
            hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        return 0;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == 1) {
            if (!game.isValidWord(pastGuesses.back().guess)) {
                MessageBoxA(hwnd, "Invalid word!", "Error", MB_OK);
                return 0;
            }

            pastGuesses.back().feedback = game.guess(pastGuesses.back().guess);
            pastGuesses.push_back({ "", {} });

            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (LOWORD(wParam) == 2) {
            MessageBoxA(hwnd, game.getTargetWord().c_str(), "Answer", MB_OK);
            return 0;
        }
        return 0;
    }

    case WM_CHAR:
    {
        char c = static_cast<char>(wParam);

        if (pastGuesses.empty() || pastGuesses.back().guess.size() < 5) {
            if (isalpha(c)) {
                pastGuesses.back().guess += tolower(c);
            }
            else if (c == '\b' && !pastGuesses.back().guess.empty()) {
                pastGuesses.back().guess.pop_back();
            }
            InvalidateRect(hwnd, NULL, TRUE); // Force a redraw after each keystroke or backspace
        }
        else if (c == '\b' && !pastGuesses.back().guess.empty()) {
            pastGuesses.back().guess.pop_back();
            InvalidateRect(hwnd, NULL, TRUE); // Force a redraw after each backspace
        }
        return 0;
    }


    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        HBRUSH brush = NULL;

        int yOffset = 0;

        // Draw past guesses
        for (const auto& gf : pastGuesses) {
            for (int i = 0; i < 5; i++) {
                rect.left = 10 + i * 60;
                rect.top = 10 + yOffset;
                rect.right = rect.left + 50;
                rect.bottom = rect.top + 50;

                brush = CreateSolidBrush(RGB(192, 192, 192));  // Default brush
                FillRect(hdc, &rect, brush);  // Clear the background

                if (i < gf.feedback.size()) {
                    std::string feedback = gf.feedback[i];

                    if (feedback == "green") {
                        brush = CreateSolidBrush(RGB(0, 255, 0));
                    }
                    else if (feedback == "yellow") {
                        brush = CreateSolidBrush(RGB(255, 255, 0));
                    }
                    else {
                        brush = CreateSolidBrush(RGB(192, 192, 192));
                    }

                    FillRect(hdc, &rect, brush);
                }

                if (i < gf.guess.size()) {
                    std::wstring wideString = std::wstring(gf.guess.begin() + i, gf.guess.begin() + i + 1);
                    SetBkColor(hdc, RGB(255, 255, 255));
                    DrawTextW(hdc, wideString.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }

                DeleteObject(brush);
            }
            yOffset += 60;
        }

        EndPaint(hwnd, &ps);
        return 0;
    }


    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "WordleWindowClass";

    WNDCLASSA wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Wordle Game",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
