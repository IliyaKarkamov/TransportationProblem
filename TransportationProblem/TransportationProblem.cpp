#include "stdafx.h"

#include "TransportProblem.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

int main()
{
    // Algorithm of success:
    // 1. Въвежда се матрицата
    // 2. Въвежда се вектор aT
    // 3. Въвежда се вектор b
    // 4. <3

    std::cout <<
        "\"Transportna zadacha po kriterii vreme\" - Algorithm by Iliya Karkamov.\n" <<
        std::endl;

    try
    {
        std::cout << "0 - Zarejdane ot konzolata\n1 - Zarejdane ot fail" << std::endl;
        auto bFromFile = false;
        std::cin >> bFromFile;

        g_bLoadedFromFile = bFromFile;
        TransportProblem problem;

        if (bFromFile)
        {
            std::cout << "Pyt kym faila: ";
            std::string sFilePath;
            std::cin >> sFilePath;

            std::ifstream file(sFilePath, std::ifstream::in);

            if (file.is_open())
            {
                file >> problem;
                file.close();
            }
        }
        else
        {
            std::cin >> problem;
        }

        std::cout <<
            "\nUspeshno vyvedohte dannite, bravo!...( piite edno kafe )" <<
            std::endl;

        const auto begin = std::chrono::steady_clock::now();
        problem.Solve();
        const auto end = std::chrono::steady_clock::now();

        std::cout << problem << std::endl;

        std::cout << "Vreme za reshavaneto na zadachata: " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() <<
            " miliseconds (" <<
            std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<
            " microseconds)" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Something went wrong, blame Microsoft.:" << std::endl;
        std::cout << e.what() << std::endl;
    }

    getchar();
    getchar();
    getchar();

    return 0;
}
