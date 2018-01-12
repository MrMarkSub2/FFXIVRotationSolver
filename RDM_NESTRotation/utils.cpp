#include "stdafx.h"
#include "utils.h"
#include <cmath>
#include <cfloat>
#include <iostream>


int calculateGCD(int ss, double huton, double haste, double arrow, double feywind, double UI3AF3)
{
    // https://www.reddit.com/r/ffxiv/comments/6oj2m1/ffxiv_secondary_stats_calculator_lvl_70/dkigx9z/
    const double basecast = 2.5;
    const int basesub = 364;
    const int lvlmod = 2170;
    return int(100 * UI3AF3 * (int(1000 * basecast * (1 - int(130 * (ss - basesub) / double(lvlmod)) / 1000.0)) / 1000.0) * (int(100 * (1 - huton) * (1 - haste) * (1 - arrow) - (100 * feywind)) / 100.0)) * 10;
}

bool IsEssentiallyEqual(double a, double b) {
    // Calculate the difference.
    double diff = abs(a - b);
    a = abs(a);
    b = abs(b);
    // Find the largest
    double largest = (b > a) ? b : a;

    if (diff <= largest * DBL_EPSILON)
        return true;
    return false;
}

int GetBufferWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    int bufferWidth, result;

    result = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufferInfo);

    if (result)
    {
        bufferWidth = bufferInfo.dwSize.X;
    }

    return bufferWidth;
}

void outputWordWrap(std::string s)
{
    int bufferWidth = GetBufferWidth();

    for (unsigned int i = 1; i <= s.length(); i++)
    {
        char c = s[i - 1];

        int spaceCount = 0;

        // Add whitespace if newline detected.
        if (c == '\n')
        {
            int charNumOnLine = ((i) % bufferWidth);
            spaceCount = bufferWidth - charNumOnLine;
            s.insert((i - 1), (spaceCount), ' ');
            i += (spaceCount);
            continue;
        }

        if ((i % bufferWidth) == 0)
        {
            if (c != ' ')
            {
                for (int j = (i - 1); j > -1; j--)
                {
                    if (s[j] == ' ')
                    {
                        s.insert(j, spaceCount, ' ');
                        break;
                    }
                    else spaceCount++;
                }
            }
        }
    }

    // Output string to console
    std::cout << s << std::endl;
}
