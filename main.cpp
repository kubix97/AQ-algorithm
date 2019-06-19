//
//  main.cpp
//  AlgorytmAQ
//
//  Copyright © 2019 Jakub Pietrowcew.
//

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream> // std::stringstream
#include <time.h>
#include <list>
#include <vector>
using namespace std;

int gRows = 136; // liczba wierszy w pliku Data.csv
int gCols = 18;  // liczba kolumn w pliku Data.csv
float gThdClass = 0.0f;// okresla punkt podzialu na klasy
int gDay1 = 5, gDay2 = 1, gDay3 = 2, gDay4 = 3; // dni z ktorych sa brane dane do treningu,
                                                //jesli chcemy symulacje dla 3 dni to gDay4 = 0

typedef list<int> TAttribute; // lista dozwolonych atrybutow w regule
typedef vector<TAttribute> TRule; // regula deycyzyjna


// zawiera rezultat oceny danej reguly
struct TRuleRes
{
    bool bCover;
    int  iClass; // 0 - negatywna, 1 - klasa ziarna
    int  iAttr; // informacja ktory atrybut z przykladu nie jest pokryty
    int  iVel;  // wartosc atrybutu niepokrytego prze regule
};



// wczytuje dane z pliku csv
// zwracane jako macierz, parametry: liczba wierszy, liczba kolumn,
// macierz zawiera tylko liczby - naglowki kolumn sa pomijane,
// tablica ma gRows-1 wierszy bo pierwszy wiersz z pliku pomijamy
float** LoadDataFromFile(int rows, int cols )
{
    
    ifstream file;
    file.open("Data.csv");
    
    if(file.good() == false)
    {
        cout << "File load error" << endl;
        //return false;
        exit(0);
    }
    
    string line; // wczytany wiersz
    string sColData; // columna danego wiersza
    float fData = 0; // dana z sColData zamieniona na float
    
    float** fTab = 0;
    fTab = new float*[rows-1];
    for (int r = 0; r < rows-1; r++)
    {
        fTab[r] = new float[cols];
    }
    
    int i = 0; int j = 0;
    getline(file, line); // piersza linia zawiera naglowki kolumn i jej nie zapisujemy
    
    while (i < rows) // czy powinno byc rows-1?
    {
        getline(file, line);
        stringstream s(line); // uzywane do podzialu wczytanego wiersza na poszczegolne kolumny
        
        while (getline(s, sColData, ';'))
        {
            
            replace(sColData.begin(), sColData.end(), ',' , '.'); // zamiana przecinka na kropke
            fData = stof(sColData); // zamiana string na float
            fTab[i][j] = fData;    // wpisanie danych do komorki
            j++;
            
            if (j == cols)
            {
                j = 0;
            }
        }
        i++;
    }
    
    file.close();
    return fTab;
}


//tworzy tablice pomocnicza ktora przechowuje dane
// o wybranych dniach do zbioru treningowego
// jedno pole tabelicy to 1 z 27 zakodowanych interwalow czasowych w danym dniu
// jeden dzien to 27 pol w tablicy
// wybranie danego pola oznacza ze ma ono w tablicy wartosc 1
// wartosci argumentow z zakresu 1 do 5 gdzie:
//poniedzialek to cyfra 1, wtorek - 2, ..., piatek - 5
int* ChooseTrainingDays(int d1, int d2, int d3, int d4)
{
    if( !(d1 != d2 && d1 != d3 && d2 != d3 && d1 != d4 && d2 != d4 && d3 != d4) ) {
        cout << "Podano błedne dni do zbioru treningowego" << endl;
        exit(0);
    }
    int* iTemp = nullptr;
    if (d4 == 0)
    {
        if( d1 < 1 || d1 > 5 || d2 < 1 || d2 > 5 || d3 < 1 || d3 > 5 ) {
            cout << "Podano błedne dni do zbioru treningowego" << endl;
            exit(0);
        }
        iTemp = new int[gRows - 1];
        for (int i =0; i < gRows - 1; i++) {
            iTemp[i] = 0;
        }
        for(int i = 0; i < 27; i++)
        {
            iTemp[(d1 - 1) * 27 + i] = 1;
            iTemp[(d2 - 1) * 27 + i] = 1;
            iTemp[(d3 - 1) * 27 + i] = 1;
        }
    }
    else
    {
        if( d1 < 1 || d1 > 5 || d2 < 1 || d2 > 5 || d3 < 1 || d3 > 5 || d4 < 1 || d4 > 5)
        {
            cout << "Podano błedne dni do zbioru treningowego" << endl;
            exit(0);
        }
        iTemp = new int[gRows - 1];
        for (int i =0; i < gRows - 1; i++) {
            iTemp[i] = 0;
        }
        for(int i = 0; i < 27; i++)
        {
            iTemp[(d1 - 1) * 27 + i] = 1;
            iTemp[(d2 - 1) * 27 + i] = 1;
            iTemp[(d3 - 1) * 27 + i] = 1;
            iTemp[(d4 - 1) * 27 + i] = 1;
        }
    }
    return iTemp;
}


// losuje 75% wierszy danych z uprzednio wybranych 3 dni
// 3 dni to 3*27 = 81 wierszy trningowych, 75% * 81 = 60.75
// Z 81 wierszy losujemy 61 wierszy
// gdy dany wiersz zostal wybrany to pole w tablicy przyjmuje wartosc 2
void SetRandomDataSet(int* iTab)
{
    srand ((unsigned) time(NULL));
    int lw = 0; // liczba wylosowanych wierszy
    int k = 0; int i = 0; // k ineks w tabeli Tab, i wirtualny indeks jedynek w Tab
    while (lw < 61)
    {
        int w = rand() % 81; // losuj wiersz treningowy z zakresu 0-80
        
        for (k = i = 0; k < gRows -1; k++)
        {
            if (iTab[k] == 0) {
                continue;
            }
            if (iTab[k] > 0)
            {
                if (i == w)
                {
                    if (iTab[k] == 1)
                    {
                        iTab[k]++; lw++;
                        break;
                    }
                }
                i++;
            }
        }
    }
}

// tworzymy listę która zawiera numery indeksów
//pod ktorymi w tablicy danych znajduja sie dane treningowe
list<int> BuildList( int* iTab)
{
    list<int> L;
    for (int i = 0; i < gRows - 1; i++) {
        /*if (iTab[i] < 2) {
            continue;
        }
         */
        if (iTab[i] == 2) {
            L.push_back(i);
        }
    }
    return L;
}


// wypisuje wczytane dane z pliku
// obecnie pierwsze 28 wierszy danych
void ShowLoadedData(float** Tab){
     for (int i=0; i<28; i++)
     {
         for (int j=0; j<18; j++)
         {
             cout << Tab[i][j] << " ";
         
             if (j == gCols-1)
             {
                 cout <<""<< endl;
             }
         
         }
     
     }
     cout << Tab[134][17] << endl;
    
}



//wypisuje wylosowane dane treningowe
void ShowTrianingTable(int* iTab)
{
    int m1 = 0;
    for (int i = 0; i< gRows - 1; i++)
    {
        cout << iTab[i] <<" ";
        if (iTab[i] == 1) {
            m1++; // zlicza liczbe jedynek - powinno byc 20 jesli sa to dane treningowe, 81 gdy testowe
        }
        if((i+1) % 27 == 0 )
        {
            cout << endl;
        }
    }
    cout << m1 << endl;
}



// wypisuje liste ktora zawiera numery wiersza treningowego
void ShowListWithIndexes (list<int> L)
{
    int l1 = 0;
    for ( int n : L) {
        l1++;
        if (l1 % 6 == 0) {
            cout << endl;
        }
        cout << n << " ";
    }
}



//inicjalizuje najbardziej ogolna regule
TRule RuleInit()
{
    TRule rule;
    TAttribute temp;
    
    for (int k = 1; k < 28; k++) {
        temp.push_back(k); // tworzymy liste zawierajaca dozwolone wartosci czasu
    }
    rule.push_back(temp); //wstawiamy wszystkie mozliwe wart I atrybutu
    temp.clear();
    
    for (int i = 0; i < 4; i++) {
        temp.push_back(i); //pozotale atrybuty przyjmuja wartosci z zakresu 0 - 3
    }
    
    for (int i = 1; i < gCols - 1; i++) //wstawiamy wszystkie mozliwe wart reszty atrybutow
    {
        rule.push_back(temp);
    }
    return rule;
}

// zapewnia ze wartosci odczytane z kolumn danych z wyjatkiem czasu beda mialy wartosci z zakresu od 0 do 3
float GetAttrVal(float* fRow, int idx)
{
    float val = 0.0f;
    if (idx > 0)
    {
        if (fRow[idx] >= 3)
        {
            val = 3.0f;
            return val;
        }
        return fRow[idx];
    }
    return fRow[idx];
}

// sprawdza czy regula pokrywa dany przyklad
TRuleRes CoverEval(TRule rule, float* dataRow)
{
    TRuleRes ruleRes;
    bool     bAttr;
    
    for (int i = 0; i < gCols - 1; i++)
    {
        int iVal = (int) GetAttrVal(dataRow, i);
        bAttr = false;
                    // look for attribute value
        for (int atr : rule[i])
        {
            if (atr == iVal) {
                bAttr = true; break;
            }
        }
        
        if( !bAttr ) {
            ruleRes.bCover = false;
            ruleRes.iAttr  = i;
            ruleRes.iClass = dataRow[gCols - 1] < gThdClass ? 0 : 1;
            ruleRes.iVel   = iVal;
            return ruleRes;
        }
    }
    return {true, dataRow[gCols - 1] < gThdClass ? 0 : 1, 0, 0};
}



//tworzy liste ktora zawiera indeksy przykladow o zadanej klasie
// parametry to: lista id rekordow treningowych, tabela danych, punkt podzialu na klase, klasa ziarna
list<int> DoListOfExamplesWithClass(list<int> TrainingList, float** fTabData, float ThdClass, int myclass )
{
    list<int> list;
    if (myclass == 1)
    {
        for (int n : TrainingList)
        {
            if (fTabData[n][17] >= ThdClass)
            {
                list.push_back(n);
            }
        }
    }
    else
    {
        for (int n : TrainingList)
        {
            if (fTabData[n][17] < ThdClass)
            {
                list.push_back(n);
            }
        }
    }
    
    return list;
}

//okresla klase do ktorej nalezy przykład na podstawie punktu podzialu ThdClass
//jesli wartosc ostaniego atrybutu w wektorze jest mniejsza od Thdclass
// to klasa  wynosi 0, w przeciwnym przypadku 1
int EvaluateClassOfRow(float* fRow, float thdClass)
{
    int Class = 0;
    if (fRow[17] >= thdClass)
    {
        Class = 1;
    }
    return Class;
}



// oblicza dokladnosc reguly jako
//stosunek pozytywnych przykladow do wszystkich pokrytych przez regule
float RuleAccurcyCalc(TRule rule, list<int> TrainingList, float** fTabData, int seedClass, float thdClass)
{
    float accurcy = 0.0f;
    int covered = 0, positive = 0;
    
    for ( int n : TrainingList)
    {
        TRuleRes temp = CoverEval(rule, fTabData[n]);
        if( temp.bCover )
        {
            
            covered++;
                        //jesli klasa pokrytego wiersza jest rowna klasie ziarna to pos++
            if ( seedClass == EvaluateClassOfRow(fTabData[n], thdClass) )
            {
                positive++;
            }
        }
    }
    accurcy = positive / (float) covered;
    return accurcy;
}


// wybiera losowy elemnt z listy
// uzyta do wybrania negatywnego przykladu xn
int GetSample(std::list<int> lst)
{
    int         v = lst.front();
    unsigned    j = (unsigned) lst.size() * rand() / RAND_MAX;
    for(auto it = lst.begin(); it != lst.end(); it++, j--) {
        if( j == 0 ) {
            v = *it; break;
        }
    }
    return v;
}



//sprawdza czy dana wartosc v jest na liscie dostepnych wartosci atrybutu
bool FindAttr(list<int> lst, int v)
{
    for(auto p : lst)
    {
        if( p == v ) {
            return true;
        }
    }
    return false;
}


// specyfikacja reguly
// regule modyfikujemy tak by pokryla ziarno, niepokryla xn
// kolejno modyfikujemy regule w jej kolumnach i za kazdym razem sprawdzamy
// czy zmiana zapewnila lepsza dokladnosc
float RuleUpdate(TRule& rl, float* fSeed, float* fNeg, list<int> lstTrn, float** fDs)
{
    float   fAccuracy = 0.0f, fTemp;
    int     iAttrCol  = -1, iSeed = EvaluateClassOfRow(fSeed, gThdClass);
    
        //przejscie po wszystkich columnach reguly
    for(int i = 0; i < rl.size(); i++)
    {
        int     iA = (int) GetAttrVal(fNeg, i);
        if( !FindAttr(rl[i], iA) )
        {       // regula nie pokrywa xn wiec brak zmian
            return RuleAccurcyCalc(rl, lstTrn, fDs, iSeed, gThdClass);
        }
        rl[i].remove(iA);   // tymczasowa modyfikacja reguly
        
                // jesli nie pokryty seed to cofnij zmiane
        if( !CoverEval(rl, fSeed).bCover ) {
            rl[i].push_back(iA);
            continue;
        }
        fTemp = RuleAccurcyCalc(rl, lstTrn, fDs, iSeed, gThdClass);
        if( fTemp > fAccuracy ) {
            iAttrCol = i; fAccuracy = fTemp;
        }
        rl[i].push_back(iA); //jesli acc nie wzroslo to cofamy zmiane
    }
                // usuwamy z reguly ta wartosc listy atrybutu ktora da max accurcy
    if( -1 != iAttrCol ) {
        int iA = (int) GetAttrVal(fNeg, iAttrCol);
        rl[iAttrCol].remove(iA);
        cout << "Mod: " << iAttrCol << " Val: " << iA << " Acc: " << fAccuracy << endl;
    }
    return fAccuracy;
}


// wybieramy ziarno o klasie zero - wartosc w kolumnnie slowness in trafic < gThdClass z listy przykladow
float* FindCorrectSeed(list<int> trainingList, float** fDataTab)
{
    for (int i : trainingList)
    {
                            //znaleziono prawidlowe
        if (EvaluateClassOfRow(fDataTab[i], gThdClass) == 0 ) {
            return fDataTab[i];
        }
    }
    return nullptr;
}


///////////////////////////////////////////////////////////////////////
//Generacja regul dla przedzialow przekazancyh w tablicy afThd
void LearnRules(vector<TRule>& vrl, float afThd[], int iN, float** fTabData)
{
    float fRuleAccurcy = -1.0f, FTempAcc = 0.0f;
    TRule outputRule;
    
    for (int m = 0; m < iN; m++)
    {
        TRule   rl, rlBest;
        gThdClass = afThd[m]; fRuleAccurcy = -1.0;
        
        cout << "Waga spowolnienia [" << m << "] " << gThdClass << " -----------------------------" << endl;
        
        for(int i = 0; i < 5; i++) // 5 razy losujemy dane i szukamy reguly o max dokladnosci
        {
            int* iTraningTab = ChooseTrainingDays(gDay1, gDay2, gDay3, gDay4);
            SetRandomDataSet(iTraningTab);
            list<int> lstTrainingRecords = BuildList(iTraningTab);
            //ShowTrianingTable(iTraningTab);
            //ShowLoadedData(fTabData);
            //ShowListWithIndexes(lstTrainingRecords);
            //cout << endl;
            rl = RuleInit();
            float* seed = FindCorrectSeed(lstTrainingRecords, fTabData);
            if( seed == nullptr )
            {
                i--;
                cout << "Powtorne szukanie odpowiedniego ziarna" << endl;
                continue;
            }
            int seedClas = EvaluateClassOfRow(seed, gThdClass);
            cout << "Ziarno: " << seedClas << " krok = " << i << " (" << lstTrainingRecords.front() << ")" <<endl;
            
            // towrzymy liste indeksow przykladow negatywnych z naszego zbioru treningowego
            list<int> lstNeg = DoListOfExamplesWithClass(lstTrainingRecords, fTabData, gThdClass, seedClas == 0 ? 1 : 0);
            cout << "Negtywne: " << endl;
            for(auto v : lstNeg) {
                cout << v << " ";
            }
            cout << endl;
            
            while( !lstNeg.empty() )
            {
                int     iNeg = GetSample(lstNeg); //losujemy przyklad negatywny xn
                float*  fNeg = fTabData[iNeg];
                
                //cout << "Sample: " << iNeg << endl;
                FTempAcc = RuleUpdate(rl, seed, fNeg, lstTrainingRecords, fTabData);
                lstNeg.remove(iNeg);
            }
            // sprawdzamy czy dla i-tych danych treningowych regula jest skuteczniejsza
            //otrzymamy zatem najdokladniejsza regule dla calego zbioru
            cout << "Regula: " << FTempAcc << endl;
            if (FTempAcc > fRuleAccurcy)
            {
                rlBest = rl ;
                fRuleAccurcy = FTempAcc;
            }
            // zwolnij pamiec
            delete iTraningTab;
        }
        
        vrl.push_back(rlBest);
                                //wypisanie m-tej reguly
        cout << endl << "Dokladnosc reguly wynikowej: " << fRuleAccurcy << " ;w przedziale: " << m+1 << endl;
        for (auto i = 0; i < rlBest.size(); i++)
        {
            cout << "Atrybut[" << i << "]: ";
            for (auto x : rlBest[i])
            {
                cout << x << " ";
            }
            cout << endl;
        }
    }

}

//////////////////////////////////////////////////////////////////
// Tworzy liste ktora zawiera numery indeksow wierszy danych,
// ktore sluza do testu otrzymanych regul
list<int> BuildTestList( int* iTab)
{
    list<int> L;
    for (int i = 0; i < gRows - 1; i++) {
        
        if (iTab[i] == 0) {
            L.push_back(i);
        }
    }
    return L;
}

//////////////////////////////////////////////////////////////////
// Sprawdzamy jak uzyskane reguly dziala na danych testowych - danych z pozostalych dni
void TestData(vector<TRule>& vrl, float afThd[], int iN, float** fTabData)
{
    int* iTraningTab = ChooseTrainingDays(gDay1, gDay2, gDay3, gDay4);
    //ShowTrianingTable(iTraningTab);
    list<int> lstTestData = BuildTestList(iTraningTab); // torzymy liste z indeksami wierszy testowych
    //ShowListWithIndexes(lstTestData);
    delete iTraningTab;
    int     iAll = 0, iGood = 0;
    
    for(auto k : lstTestData)
    {
        float*  fT = fTabData[k];
        int     i, j, l;
                    // znajdz pierwsza regule pokrywajaca
        for(i = 0; i < vrl.size(); i++)
        {
            if( CoverEval(vrl[i], fT).bCover ) {
                break;
            }
        }
                    // wyznacz indeks przedzialu spowolnienia
        for(j = 0, l = iN; j < iN; j++)
        {
            if( fT[gCols - 1] < afThd[j] )
            {
                l = j; break;
            }
        }
        iGood += (i == l ? 1 : 0); //jesli regula pokrywa odpowiadajacy jej przedzial to dobrze
        iAll++;
    }
    cout << "Liczba prawidlowo pokrytch przykladow testowych: " << iGood << endl;
    cout << "Liczba wszystkich przykladow testowych: " << iAll << endl;
    cout << "Procent prawidlowo przydzielonych " << (iGood/(double)iAll)*100 << "%" << endl;
}

//////////////////////////////////////////////////////////////////
int main(int argc, const char * argv[])
{
    if( argc > 4 ) {
        gDay1 = atoi(argv[1]); gDay2 = atoi(argv[2]); gDay3 = atoi(argv[3]); gDay4 = atoi(argv[4]);
    }
    else
    if( argc > 3 ) {
        gDay1 = atoi(argv[1]); gDay2 = atoi(argv[2]); gDay3 = atoi(argv[3]); gDay4 = 0;
    }
    
    float thdClassTab [] = { 5.0f, 15.0f }; // wartosci slowness in trafic w danych wejsciowych ktore sa mniejsze otrzymuja klase 0
    vector<TRule> vectorWithResultRules; //wektor na reguły wynikowe pokrywajace dane przedzialy 5 procentowe
    float** fTabData = LoadDataFromFile(gRows, gCols);
    
    LearnRules(vectorWithResultRules, thdClassTab, sizeof(thdClassTab) / sizeof(float), fTabData);
    
    if( sizeof(thdClassTab) / sizeof(float) == vectorWithResultRules.size() )
    {
        TestData(vectorWithResultRules, thdClassTab, sizeof(thdClassTab) / sizeof(float), fTabData);
    }
    else
    {
        cout << "Nie wygenerowano reguł dla wszystkich przedzialow" << endl;
    }
    
    return 0;
}
