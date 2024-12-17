#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>
#include <fstream>
#include <windows.h>

#include <codecvt> // Для работы с codecvt_utf8

using namespace std;

vector<wstring> vec;

wstring trim(const wstring &str)
{
    size_t start = str.find_first_not_of(L" \t\r\n");
    size_t end = str.find_last_not_of(L" \t\r\n");
    return (start == wstring::npos) ? L"" : str.substr(start, end - start + 1);
}

void readArray()
{
    wifstream f(L"russian-nouns.txt");                    // поток std::wifstream для чтения std::wstring
    f.imbue(locale(locale(), new codecvt_utf8<wchar_t>)); // Конвертер UTF-8 -> wstring
    wstring str;
    while (getline(f, str))
    {
        vec.push_back(trim(str));
    }
}

bool wordExists(const wstring &playerWord)
{
    return find(vec.begin(), vec.end(), playerWord) != vec.end();
}

class Points
{
public:
    int pointsScoring(const wstring &mainWrd, const wstring &playerWord, const vector<wstring> &pastWords)
    {
        // Пропуск хода
        if (playerWord.empty())
        {
            return 0;
        }

        // Проверка на повторные слова
        for (const wstring &pastWord : pastWords)
        {
            if (playerWord == pastWord)
            {
                // Слово уже было использовано, штраф = -длина слова
                return -(int)playerWord.size();
            }
        }

        // Проверка на существование слова (пока возвращает true)
        if (!wordExists(playerWord) || (playerWord == mainWrd))
        {
            // Если слово не существует, штраф = -длина_слова
            return -(int)playerWord.size();
        }

        // Подсчёт совпадений букв при помощи частот
        map<char, int> freqMain;
        map<char, int> freqPlayer;

        // Считаем вхождения букв в базовом слове
        for (char c : mainWrd)
        {
            freqMain[c]++;
        }

        // Считаем вхождения букв в слове игрока
        for (char c : playerWord)
        {
            freqPlayer[c]++;
        }

        // Подсчитываем количество совпавших букв
        int counterRight = 0;
        for (auto &pair : freqPlayer)
        {
            char c = pair.first;
            int countPlayer = pair.second;
            int countMain = freqMain[c];
            // Количество совпадений для этой буквы - минимум из двух частот
            counterRight += min(countPlayer, countMain);
        }

        int counterError = (int)playerWord.size() - counterRight;
        int score = 0;
        // Если все буквы совпали, очки = длина слова
        // Иначе - штраф за несовпадение
        if (counterRight == (int)playerWord.size())
        {
            score = counterRight;
        }
        else
        {
            score = -counterError;
        }

        return score;
    }
};

class WordGame
{
private:
    wstring baseWord;          // Базовое слово
    vector<wstring> usedWords; // Список использованных слов
    vector<int> scores;        // Очки каждого игрока
    vector<int> skipCount;     // Счётчик пропусков для каждого игрока
    int numPlayers;            // Количество игроков
    bool gameActive;           // Флаг активности игры
    int consecutiveSkips;      // Счётчик последовательных пропусков
    Points playerPoints;       // Объект для подсчёта очков

public:
    // Конструктор
    WordGame(const wstring &base, int players)
        : baseWord(base), numPlayers(players), gameActive(true), consecutiveSkips(0)
    {
        skipCount.resize(numPlayers, 0);
        scores.resize(numPlayers, 0);
    }

    // Запуск игры
    void start()
    {
        wcin.ignore(); // Игнорируем символ новой строки после ввода количества игроков
        while (gameActive)
        {
            for (int i = 0; i < numPlayers; ++i)
            {
                playerTurn(i);
                if (!gameActive)
                    break;
            }
        }
        endGame();
    }

private:
    void playerTurn(int playerIndex)
    {
        wcout << L"Ход игрока " << (playerIndex + 1) << L": ";
        wstring inputWord;
        getline(wcin, inputWord);
        wstring playerWord = trim(inputWord);

        // Если пропуск хода
        if (playerWord.empty())
        {
            wcout << L"Игрок " << (playerIndex + 1) << L" пропустил ход.\n";
            skipCount[playerIndex]++;
            consecutiveSkips++;
            if (consecutiveSkips == numPlayers)
            {
                // Все игроки подряд пропустили
                gameActive = false;
            }
            return;
        }

        // Подсчёт очков за введённое слово
        int score = playerPoints.pointsScoring(baseWord, playerWord, usedWords);
        if (score > 0)
        {
            // Слово корректно составлено
            scores[playerIndex] += score;
            usedWords.push_back(playerWord);
            skipCount[playerIndex] = 0;
            consecutiveSkips = 0;
            wcout << L"Слово принято! Очки: +" << score << L" всего: " << scores[playerIndex] << L"\n\n";
        }
        else if (score < 0)
        {
            // Ошибка: слово повторное, не существует или нельзя составить корректно
            scores[playerIndex] += score;
            skipCount[playerIndex] = 0;
            consecutiveSkips = 0;
            wcout << L"Некорректное слово. Очки: " << score << L" всего: " << scores[playerIndex] << L"\n\n";
        }
        else
        {
            // Теоретически сюда попасть нельзя при корректной логике
            skipCount[playerIndex] = 0;
            consecutiveSkips = 0;
            wcout << L"Слово не даёт очков.\n";
        }
    }

    // Завершение игры
    void endGame()
    {
        wcout << L"Игра окончена!\n";
        wcout << L"Очки игроков:\n";

        int maxScore = -999999;
        int winner = 0;              // Номер победителя (индекс игрока)
        bool allEqual = true;        // Флаг, что все очки одинаковы
        bool tieBetweenSome = false; // Флаг для ничьей между несколькими игроками

        // Проверяем очки
        for (int i = 1; i < numPlayers; i++)
        {
            if (scores[i] != scores[0])
                allEqual = false; // Если хотя бы одно значение отличается, это не полная ничья
        }

        // Если все очки равны
        if (allEqual)
        {
            wcout << L"Ничья! Все игроки набрали по " << scores[0] << L" очков.\n";
            return;
        }

        // Определяем максимальный счет и ищем ничью между несколькими игроками
        int countMax = 0; // Счетчик количества игроков с максимальными очками
        for (int i = 0; i < numPlayers; i++)
        {
            if (scores[i] > maxScore)
            {
                maxScore = scores[i];
                winner = i + 1; // Обновляем победителя
                countMax = 1;   // Сбрасываем счетчик ничьей
                tieBetweenSome = false;
            }
            else if (scores[i] == maxScore)
            {
                countMax++;            // Увеличиваем счетчик игроков с максимальными очками
                tieBetweenSome = true; // Фиксируем ничью между несколькими игроками
            }
        }

        // Проверяем на ничью между несколькими игроками
        if (countMax > 1)
        {
            wcout << L"Ничья между несколькими игроками с " << maxScore << L" очками.\n";
        }
        else
        {
            // Победитель найден
            wcout << L"Победил игрок " << winner << L" с " << maxScore << L" очками!\n";
        }
    }
};

int main()
{
    // Установим кодировку CP1251 для корректной работы с русскими символами
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian");

    readArray();
    int numPlayers = -1;
    while (numPlayers == -1)
    {
        wcout << L"Введите количество игроков (от 2 до 4): ";
        wcin >> numPlayers;
        if (numPlayers > 4 || numPlayers < 2 || numPlayers == -1)
        {
            wcout << L"Игроков должно быть от 2 до 4\n";
            numPlayers = -1;
            wcin.clear();
            wcin.ignore(10000, '\n');
        }
    }

    wstring baseWord;
    wcout << L"Введите базовое слово: ";
    wcin >> baseWord;

    WordGame game(baseWord, numPlayers);
    game.start();

    cin.get();

    return 0;
}