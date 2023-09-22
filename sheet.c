/********************************************************************************************************************************************************/
/*                                                          Projekt 1 - práce s textem                                                                  */
/*                                                                                                                                                      */
/*                                                                                                                                                      */
/*                                                                   Verze: 1.0                                                                         */
/*                                                              Autor: Žalmánek Matěj                                                                   */
/*                                                               VUT login: xzalma00                                                                    */
/*                                                           Datum: Říjen - Listopad 2020                                                               */
/*                                                                                                                                                      */
/*                                                                                                                                                      */
/********************************************************************************************************************************************************/


/********************************* Mapa kódu: *****************************************************/
/***************** line 47 - Deklarace vlastních datových typů ************************************/
/***************** line 64 - Pomocné funkce pro řešení jednotlivých příkazů ***********************/
/***************** line 644 - Příkazy pro úpravu velikosti tabulky ********************************/
/***************** line 751 - Obsluhy příkazů - Povinné příkazy ***********************************/
/***************** line 919 - Obsluhy příkazů - Volitelné příkazy *********************************/
/***************** line 1165 - Příkazy pro zpracování argumentů ***********************************/

#define ERR_WRONG_INTERVAL "Zadany interval je spatne zadany! Prvni cislo musi byt mensi!\n"
#define ERR_WRONG_SELECTION_ROW "Radek musi byt vetsi nez 0!\n"
#define ERR_WRONG_SELECTION_COL "Sloupec musi byt vetsi nez 0!\n"
#define ERR_LONG_ARG "Zadany argument je prilis dlouhy! Maximalni delka argumentu je 100 znaku\n"
#define ERR_MISSING_PARAMETER "Zapomneli jste na zadani povinnych argumentu prikazu!\n"
#define ERR_PARAMETER_IS_NOT_NUM "V zadanych argumentech se vyskytuje chyba! Ocekaval jsem cislo, ale v argumentu je jiny format!\n" 
#define ERR_EMPTY_FILE "Soubor je prazdny!\n"
#define ERR_LONG_LINE "Prilis mnoho znaku na radce!\n"
#define ERR_COL_OUT_OF_LINE_RANGE "Sloupec je mimo hranice počtu sloupců v řádku!!\n"
#define ERR_COL_IN_INTERVAL "Cislo sloupce se vyskytuje v zadaném intervalu!\n"
#define ERR_IN_COL_IS_NOT_NUM "Ve sloupci ktery se mel zpracovavat se ocekavalo cislo, ale to tam nebylo!\n"
#define LINE_MAX_LENGTH 10*1024
#define NUMBER_ROW_COMMANDS 4
#define DELIM_LENGTH 101
#define MAX_LENGTH_FLOAT_STRING 15
#define MAX_LENGTH_INT_STRING 11

//Includované knihovny
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>


/***************************************************** Deklarace vlastních datových typů *************************************************************************/
enum LineOperations{IROW=0,AROW,DROW,DROWS};

//Pro příkazy, které nějak pracují s daty v řádcích (např. rargv - aritmetický průměr hodnot ze sloupce na řádcích od M do N)
typedef struct SharedVariables {
    double vysledek;
    int pocet;
}SharedVariables;

//Struktura, která sdružuje argumenty pro příkazy pracující s intervalama a výběrem sloupce
typedef struct{
    int colSelect;
    int intervalStart;
    int intervalEnd;
} ArgsOfIntervalCommands;

/***************************************************** Pomocné funkce pro řešení jednotlivých příkazů *************************************************************************/

/* Vypisuje chybu na stderr
 * @param {char} error - Text, který se má vypsat */
void printError(char *error) {
    fprintf(stderr,"Omlouvame se, ale nastala chyba: %s",error);
}

/* Validuje, jestli počet argumentů z terminálu je dostačijící */
int validateNumOfParams(int numParams,int posToRead,int argc)
{
    if(numParams+posToRead>argc) {
        printError(ERR_MISSING_PARAMETER);
        return 0;
    }
    return 1;
}

/* Validuje správnost intervalu (Pouze pro účely zadání, matematicky to je špatně <a,a> není platný interval) */
int validateInterval(int startNum,int endNum)
{
    if(startNum>endNum) {
        printError(ERR_WRONG_INTERVAL);
        return 0;
    }
    return 1;
}

/* Validuje, zda sloupec, který chce uživatel vybrat je větší než 0 */
int validateColSelect(int colNum)
{
    if(colNum<=0){
        printError(ERR_WRONG_SELECTION_COL);
        return 0;
    }
    return 1;
}

/* Validuje, zda řádek, který chce uživatel vybrat je větší než 0 */
int validateRowNum(int num) {
    if(num<=0){
        printError(ERR_WRONG_SELECTION_ROW);
        return 0;
    }
    return 1;
}

/* Validuje správnost pozice ve stringu, implementováno tak, že pokud vyjde -1, ve sloupci nebylo číslo i když bylo požadováno */
int validateStrPos(int posCol) {
    if(posCol==-1){
        printError(ERR_IN_COL_IS_NOT_NUM);
        return 0;
    }
    return 1;
}

/* Validuje, zda číslo je uvnitř intervalu, pokud ano vrací 1 */
int validateNumInInterval(int startNum,int endNum,int validateNum)
{
    if(validateNum>=startNum&&validateNum<=endNum){
        printError(ERR_COL_IN_INTERVAL);
        return 1;
    }
    else return 0;
}

/* Zjistí, zda znak je obsazen v řetězci delim */
bool isDelim(char znak,char *delim) {
    for(int i=0;delim[i]!='\0';++i)
        if(delim[i]==znak)return true;
    return false;
}

/**
 * Vrací počáteční pozici sloupce v řetězci line
 * 
 * @param {int} col - Číslo sloupce, který má být nalezen v rámci řetězce
 * @return {int} - Vrací pozici sloupce v řetězci, v případě že sloupec v řetězci neexistuje, vrací -1 a vypíše error
*/
int getPosOfCol(char *line,int col,char *delim) {
    if(col==1)return 0;
    int pocetSloupcu=1;
    for(int i=0;line[i]!='\0';++i)
    {
        if(isDelim(line[i],delim)){
            ++pocetSloupcu;
            if(pocetSloupcu==col)return i+1;
        }
    }
    printError(ERR_COL_OUT_OF_LINE_RANGE);
    return -1;
}

/**
 * Vloží znak doprostřed řetězce a celý řetězec tak posune o jednu pozici dále 
 * @param {char *} string - Řetězec, do kterého se má znak vložit
 * @param {int} stringMaxLen - Určuje maximální velikost řetězce, kterou se nesmí překročit
 * @param {znak} znak - Znak, který má být vložen
 * @param {znak} pozice - Pozice ve stringu, na kterou se má znak vložit 
*/
bool vlozitZnakDoStringu(char *string,int stringMaxLen,char znak,int pozice) {
    int delkaString=strlen(string);
    if(delkaString+2>stringMaxLen)return false; //+2 jedno za \0 a další za nový znak
    char predchoziZnak=string[pozice];
    char aktualniZnak;
    string[pozice]=znak;
    while(string[pozice]!='\0')
    {
        ++pozice;
        aktualniZnak=string[pozice];
        string[pozice]=predchoziZnak;
        predchoziZnak=aktualniZnak;
    }
    string[pozice]='\0';
    return true;
}
/**
 * Zjistí, zda string od určité pozice po určitou pozici obsahuje znak znak
 * 
 * @param {int} startpozice - pozice, na které se má začínat hledat
 * @param {int} endpozice - pozice, na které se hledání ukončuje
 * @param {char *} string - Řetězec, ve kterém se má hledat znak
 * @param {char} znak - Znak, který má být nalezen
*/
bool containsChar(int startpozice,int endpozice,char *string, char znak) {
    for(int i=startpozice;i<endpozice&&string[i]!='\0';++i)
    {
        if(string[i]==znak)return true;
    }
    return false;
}

//Kontroluje, zda určitý string je číslo
bool kontrolaIsCislo(char *string) {
    int pocetDesCarek=0;
    int pocetMocnin10=0;
    for(int i=0;string[i]!='\0';++i)
    {
        if(string[i]=='.'){
            if(pocetDesCarek>0)return false;
            ++pocetDesCarek;
        }
        else if(string[i]=='e'){
            if(pocetMocnin10>0)return false;
            ++pocetMocnin10;
        }
        else if((string[i]-'0')>9||string[i]-'0'<0)return false;
    }
    return true;
}

/**
 * Vrátí argument z argv na určité pozici a tuto pozici v argv zvětší o 1
 * 
 * @param {int *} pozice - Ukazatel na pozici v argv, ze které se má načíst argument, tato pozice bude zvětšena o 1
 * @param {int} error - Indikuje v případě, že se volá více náčítání argumentů za sebou, zda v předchozím volání nastala chyba 
 * @return {char *} - Pokud se vše povede, vrací ukazatel na místo v paměti, kde je daný argument, pokud se to nepovede, vrací NULL a vypíše chybu
*/
char *NactiDalsiArgument(int argc,char *argv[],int *pozice,int error) {
    if(error==-1)return NULL;//Pokud je funkce volána vícekrát za sebou, je zbytečné několikrát vypisovat stejný error

    if(!validateNumOfParams(1,*pozice,argc)) return NULL;
    else {
        ++(*pozice);
        return argv[*pozice];
    }
}

/**
 * Vrátí číslo získané z řetězce v argv na určité pozici v argv
 * 
 * @param {int *} pozice - Ukazatel na pozici v argv, ze které se má načíst argument, tato pozice bude zvětšena o 1
 * @param {int} error - Indikuje v případě, že se volá více náčítání argumentů za sebou, zda v předchozím volání nastala chyba
 * @return {int} - Pokud se vše povede, vrátí se zkonvertované číslo, pokud ne, vrátí se -1
*/
int NactiDalsiArgumentToInt(int argc,char *argv[],int *pozice,int error) {
    if(error==-1)return -1;
    char *cislo=NactiDalsiArgument(argc,argv,pozice,false);
    if(!kontrolaIsCislo(cislo)){
        printError(ERR_PARAMETER_IS_NOT_NUM);
        return -1;
    }
    if(cislo!=NULL)return atoi(cislo);
    else return -1;//O error log se stará už NactiDalsiArgument()
}

/**
 * Vrátí číslo v double získané z řetězce v argv na určité pozici v argv
 * 
 * @param {int *} pozice - Ukazatel na pozici v argv, ze které se má načíst argument, tato pozice bude zvětšena o 1
 * @param {int} error - Indikuje v případě, že se volá více náčítání argumentů za sebou, zda v předchozím volání nastala chyba
 * @return {double} - Pokud se vše povede, vrátí se zkonvertované číslo, pokud ne, vrátí se -1
*/
double NactiDalsiArgumentToDouble(int argc,char *argv[],int *pozice,int error) {
    if(error==-1)return -1;
    char *cislo=NactiDalsiArgument(argc,argv,pozice,false);
    if(cislo!=NULL)
    {
        if(!kontrolaIsCislo(cislo)){
            printError(ERR_PARAMETER_IS_NOT_NUM);
            return -1;
        }
        else return atof(cislo);
    }
    else return -1;//O error log se stará už NactiDalsiArgument()
}

/**
 * Maže znaky na pozici ve stringu z intervalu <zacatek,konec) - přesune znaky ze stringu za pozicí konec na pozici zacatek a poté ukončí znakem '\0' 
 * 
 * @param {char *} string - Řetězec, ve kterém se má mazat
 * @param {int} zacatek - Číslo označující počáteční pozici ve stringu ke smazání (včetně)
 * @param {int konec} - Číslo označující koncovou pozici ve stringu ke smazání (bez)
*/
void posunVeStringu(char *string,int zacatek,int konec) {
    for(int i=konec;string[i]!='\0';++i){
        string[zacatek]=string[i];
        ++zacatek;
    }
    string[zacatek]='\0';
}

/* Odstraní právě jeden znak ze stringu na určité pozici */
void removeCharFromString(char *string,int pozice) {
    posunVeStringu(string,pozice,pozice+1);
}

/**
 * Smaže všechen text ze sloupce, který načíná ve stringu na určité pozici, vrací počet, kolik znaků smazal 
 * 
 * @param {char *} string - Řetězec, ze kterého se má text sloupce smazat
 * @param {int} pozice - Pozice v řetězci, kde začíná sloupec, kterého text se má mazat
 * @param {char *} delim - Řetězec, reprezentující jednotlivé znaky, které jsou považovány za oddělovače jednotlivých sloupců
 * @return {int} - Počet znaků, které funkce smazala
*/
int smazTextSloupce(char *string,int pozice,char *delim) {
    int smazano=0;
    while(string[pozice]!='\0'&&string[pozice]!='\n')
    {
        if(isDelim(string[pozice],delim))break;
        removeCharFromString(string,pozice);
        ++smazano;
    }
    return smazano;
}

/**
 * Přesune prvky za určitým prvkem v poli před tento prvek a sníží číslo určující velikost pole (Optimalizace procházení prvků v poli) 
 *
 * @param {int *} velikost - Ukazatel na číslo reprezentující velikost pole, toto číslo se nakonec sníží
 * @param {char *[]} pole - Pole řetězců, ve kterém se mají prvky přesouvat o jednu pozici dopředu od dané pozice v poli 
 * @param {int *} pozice - Ukazatel na číslo reprezentující pozici v poli řetězců, od které se mají následující prvky pole přesouvat o
 *                         jednu pozici dopředu, tato pozice se nakonec sníží o jedno z důvodu procházení dalších prvků v poli bez přeskočení prvku
 *                         na pozici: pozice+1
*/
void presunVPoli(int *velikost,char *pole[],int *pozice) {
    for(int i=*pozice;i<((*velikost)-1);++i)
        pole[i]=pole[i+1];
    (*velikost)--;
    (*pozice)--;
}

/**
 * Upraví delim tak, aby se neopakovali stejné znaky nikde v řetězci a zkopíruje lokální delim do hlavního delimu
 * 
 * @param {char *} delimArgv - Řetězec, ze krerého se má delim nastavit
*/
void upravDelim(char *delimArgv,char *delim) {
    int pocetpripsani=0;
    int length=strlen(delimArgv);
    
    for(int i=0;delimArgv[i]!='\0';++i)
    {
        if(!containsChar(i+1,length,delimArgv,delimArgv[i])){
            delim[pocetpripsani]=delimArgv[i];
            ++pocetpripsani;
        }
    }
    delim[pocetpripsani]='\0';
}

/**
 * Vrací již upravený řetězec delim
 * 
 * @param {int *} argc - Ukazatel na číslo reprezentující velikost pole argv, toto číslo bude zmenšeno o počet argumentů,které nastavují delim
 * @param {char *[]} argv - Pole argumentů z terminálu
 * @return {char *} - Vrací řetězec delim zadaný v argumentu
*/
void setDelim(char *delim,int *argc,char *argv[]) {
    delim[0]=' ';
    delim[1]='\0';
    for(int i=0;i<*argc;++i)
    {
        if(strcmp(argv[i],"-d")==0){
            char *delimArgv=NactiDalsiArgument(*argc,argv,&i,false);
            if(delimArgv!=NULL){
                upravDelim(delimArgv,delim);
                presunVPoli(argc,argv,&i);
                presunVPoli(argc,argv,&i);
            }else delim[0]='\0';
            return;
        }
    }
}

/**
 * Nalezne minimální prvek v poli od určité pozice a vrátí minimum 
 * 
 * @param {int *} array - Pole čísel, ve kterém se má hledat minimum
 * @param {int} delkaArr - Velikost pole čísel, ve kterém se má hledat minimum
 * @param {int} pos - pozice, od které se má začít hledat minimum v poli
 * @param {int *} minPos - Ukazalel na číslo reprezentující pozici v poli, na které se minimum v tomto poli nachází
 * @return {int} - vrací minimální číslo z pole
*/
int findMin(int *array,int delkaArr,int pos,int *minPos) {
    int min=array[pos];
    *minPos=pos;
    for (int i=pos;i<delkaArr;++i)
    {
        if(array[i]<min){
            min=array[i];
            *minPos=i;
        }
    }
    return min;
}

/* Seřadí vzestupně prvky v poli */
void sortArray(int arrayToSort[],int delkaArr) {
    int tmp;
    for(int i=0;i<delkaArr;++i)
    {
        int minPos;
        int min=findMin(arrayToSort,delkaArr,i,&minPos);
        tmp=arrayToSort[i];
        arrayToSort[i]=min;
        arrayToSort[minPos]=tmp;
    }
}

/* Přesune všechny prvky pole o jednu pozici "doleva", první prvek se přepíše */
void arrMoveAllToLeft(int pole[]) {
    for(int i=0;pole[i]!=0;++i)
        pole[i]=pole[i+1];
}

/**
 * Najde pozici příkazu pro selekci řádků 
 * 
 * @return {int} - Vrací pozici příkazu pro selekci řádků v poli s řetězci, pokud příkaz pro selekci řádků nenajde, vrací -1
*/
int getPosOfRowSelectCommand(int argc,char *argv[]) {
    for(int i=0;i<argc;++i)
    {
        if(strcmp(argv[i],"rows")==0)return i;
        else if(strcmp(argv[i],"contains")==0)return i;
        else if(strcmp(argv[i],"beginswith")==0)return i;
    }
    return -1;
}

/**
 * Připíše do stringu na určitou pozici string
 * 
 * @param {int} stringMaxLen - Určuje maximální velikost řetězce, kterou se nesmí překročit
 * @param {int} pozice - Pozice v řetězci, na kterou se má přidat string
*/
void nastavString(char *string,int stringMaxLen,int pozice,char *nastav) {
    for(int i=0;nastav[i]!='\0';++i)
        vlozitZnakDoStringu(string,stringMaxLen,nastav[i],pozice+i);
}

/* Převede znak na určité pozici uvnitř stringu z Velkého písmena na malé písmeno */
void charToLower(int pozice,char *string) {
    if(string[pozice]>='A'&&string[pozice]<='Z')
        string[pozice]=string[pozice]+('a'-'A');
}

/* Převede znak na určité pozici uvnitř stringu z malého písmena na Velké písmeno */
void charToUpper(int pozice,char *string){
    if(string[pozice]>='a'&&string[pozice]<='z') 
        string[pozice]=string[pozice]-('z'-'Z');
}

/**
 * hledá ve stringu následující jednoho ze znaků z řetězce delim a to od určité pozice
 * 
 * @param {char *} delim - Řetězec, obsahující všechny možné oddělovače buňek (sloupců)
 * @param {int} pozice - Pozice, od které se má začít hledat
 * @return {int} - Vrací pozici prvního nalezeného znaku delim, popřípadě pozici '\n' || '\0' = konec posledního sloupce na řádku
*/
int findNextDelim(char *string, char *delim,int pozice) {
    while(string[pozice]!='\n'&&string[pozice]!='\0')
    {
        if(isDelim(string[pozice],delim)) return pozice;
        ++pozice;
    }
    return pozice;
}

/* Zjistí délku stringu uvnitř sloupce */
int getSizeOfColString(char *line,char *delim,int posCol){
    int endPosCol=findNextDelim(line,delim,posCol)-1;
    return endPosCol-posCol+1;
}

/**
 * Vrací substring z pozice ve stringu <start,end> 
 * 
 * @param {int} start - Počáteční pozice ze stringu, ze které se má získat substring
 * @param {char[]} substringArr - Pole charů, sloužící pro uložení substringu (do pole lze číst i zapisovat)
 * @param {int} velikostPole - Určuje velikost pole substringArr
 * @return {char *} - Vrací ukazatel na začátek pole znaků substringArr  
*/
char *getSubstring(int start,char *string,char substringArr[],int velikostPole) {
    char *substring=substringArr;
    int end=start+velikostPole-1;
    for(int i=start;string[i]!='\0'&&i<=end;++i)
    {
        substring[i-start]=string[i];
    }
    substring[velikostPole]='\0';
    return substring;
}

/* Zjistí, zda string obsahuje substring contains */ 
bool containsStr(char *string, char *contains) {
    for(int i=0;string[i]!='\0';++i)
    {
        if(string[i]==contains[0]){
            bool obsahuje=true;
            for(int j=1;contains[j]!='\0';++j)
            {
                if(string[i+j]!=contains[j])obsahuje=false;
            }
            if(obsahuje)return true;
        }
    }
    return false;
}

/* Zjišťuje, zda string začíná substringem startsWith */
bool startsWithStr(char *string, char *startsWith) {
    for(int i=0;string[i]!='\0';++i){
        if(startsWith[i]=='\0')return true;
        if(startsWith[i]!=string[i])return false;
    }
    return false;
}

/**
 * Najde další pozici výskytu znaku find ve stringu od určité pozice
 * 
 * @return - Vrací pozici znaku find pokud se zadaný znak podaří najít, v opačném případě vrací -1
*/
int nextPositionOf(char find,char *string,int startPosition) {
    while(string[startPosition]!='\0')
    {
        if(string[startPosition]==find)return startPosition;
        ++startPosition;
    }
    return -1;
}

/* Ze stringu vytvoří zaokrouhlené číslo, které taky vrátí */
int my_floor(char *cisloStr) {
    float cislof=atof(cisloStr);
    int intCislo;
    snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",cislof);//Pokud je číslo formátu 4.15e1, aby se čárka posunula
    if(containsStr(cisloStr,".")==0){ //Pokud neobahuje desetinné místo, aby nehledal desetinny
        intCislo=(int) cislof;
    }
    else {
        int poziceTecka=nextPositionOf('.',cisloStr,0);
        int desetiny=cisloStr[poziceTecka+1]-'0';
        intCislo=(int) cislof;
        if(poziceTecka==0)intCislo=0;//.4
        if(desetiny>=5)++intCislo;
    }
    return intCislo;
}

/**
 * Convertuje číslo typu int na pole znaků
 * @param {int} cislo - Číslo, které se má convertovat
 * @param {char[]} stringArr - Pole znaků, do kterého se má vložit string, který vznikl converzí čísla
 * @return - vrací ukazatel na začátek pole znaků
*/ 
char *intToString(int cislo,char stringArr[]) {
    char *string=stringArr;
    if(cislo==0){
        string[0]='0';
        string[1]='\0';
    }
    else string[0]='\0'; /* Ukončení čísla int pro další práci s tímto stringem v jiných funkcích
    *                    '\0' si bude postupně posouvat na konec */
    for(int i=0;cislo>0;++i)
    {
        int cifra=cislo%10;
        cislo/=10;
        vlozitZnakDoStringu(string,MAX_LENGTH_INT_STRING,(cifra+'0'),0);
    }
    return string;
}

/* Zkopíruje jeden řetězec do druhého řetězce (pole charů) */
void copyStrToStr(char *strToCopy,char *strCopyTo) {
    int i;
    for(i=0;strToCopy[i]!='\0';++i)
    {
        strCopyTo[i]=strToCopy[i];
    }
    strCopyTo[i]='\0';
}

/* Přečte číslo ze sloupce daného určitou pozicí ve stringu */
double readNumFromCol(char *line,int *pozice,char *delim) {
    int velikostPole=getSizeOfColString(line,delim,(*pozice));
    char substringArr[velikostPole];
    char *substring=getSubstring((*pozice),line,substringArr,velikostPole);
    if(!kontrolaIsCislo(substring)){
        *pozice=-1;
        return 1;
    }
    float cislo=atof(substring);
    (*pozice)+=velikostPole;
    return cislo;
}

/* Odstraní sloupec začínající na určité pozici ve stringu */
int removeCol(char *string,char *delim,int colPos)
{
    int smazano=smazTextSloupce(string,colPos,delim);
    if(isDelim(string[colPos],delim))removeCharFromString(string,colPos);
    else removeCharFromString(string,colPos-1);
    return smazano+1;
}

/* Vloží sloupec na nějakou pozici ve stringu */
void insertCol(char *string,char *newColString,int posToInsert,char *delim)
{
    vlozitZnakDoStringu(string,LINE_MAX_LENGTH,delim[0],posToInsert);
    nastavString(string,LINE_MAX_LENGTH,posToInsert,newColString);
}

/* Nastaví defaultní hodnoty do sdílené proměnné sharedVariables mezi rxxx příkazy (pro ty, které potřebují změnit defaultní hodnotu 0 = rseq,rmax,rmin) */
void sharedVarSetDefaultValue(SharedVariables *sharedVariables,int set)
{
    if(sharedVariables->pocet==0){
        sharedVariables->vysledek=set;
        sharedVariables->pocet++;
    }
}

//Nahradí všechny oddělovače prvním oddělovačem z řetězce delim
void replaceAllDelims(char *line,char *delim){
    if(delim[1]=='\0'||delim[0]=='\0') return;
    for(int i=0;line[i]!='\0';++i)
    {
        if(isDelim(line[i],delim))line[i]=delim[0];
    }
}

/***************************************************** Příkazy pro úpravu velikosti tabulky *************************************************************************/

/**
 * vloží prázdný sloupec před sloupec daný číslem 
 * 
 * @param {int} sloupec - Číslo sloupce, před který se má vkládat prázdný sloupec
 * @param {char *} string - Řetězec, do kterého se má vkládat prázdný sloupec
 * @return {int} - Vrací hodnotu určující, zda se příkaz povedl vykonat
*/
int icol(int sloupec,char *string,char *delim) {
    if(!validateColSelect(sloupec)) return 1;
    int pocetSloupcu=1;
    bool prvniPruchod=true;
    for(int i=0;string[i]!='\0';++i)
    {
        if(prvniPruchod){
            if(pocetSloupcu==sloupec) vlozitZnakDoStringu(string,LINE_MAX_LENGTH,delim[0],i);
            prvniPruchod=false;
        }
        if(isDelim(string[i],delim)){
            ++pocetSloupcu;
            prvniPruchod=true;
        }
    }
    return 0;
}

// Přidá prázdný sloupec na konec řetězce
void acol(char *line, char *delim) {
    int lastPos=strlen(line)-1;
    vlozitZnakDoStringu(line,LINE_MAX_LENGTH,delim[0],lastPos);
}

/**
 * Smaže sloupec daný číslem z řetězce
 * 
 * @param {int} sloupec - Číslo označující číslo sloupce v řetězci
 * @param {char *} string - Řetězec, ve kterém se má smazat sloupec
 * @return {int} - Vrací hodnotu určující, zda se příkaz povedl vykonat
*/
int dcol(int sloupec,char *string, char *delim) {
    if(!validateColSelect(sloupec)) return 1;
    int pocetSloupcu=1;
    bool prvniPruchod=true;
    for(int i=0;string[i]!='\0';++i)
    {
        if(prvniPruchod){
            if(pocetSloupcu==sloupec) {
                smazTextSloupce(string,i,delim);
                if(string[i]!='\n')removeCharFromString(string,i); //Smazání znaku delimu, aby se smazal celý sloupec a ne jenom jeho text
                else if(i-1>0){ //Pokud se smazaly všechny řádky a zbývá poslední znak '\n'
                        removeCharFromString(string,i-1); //Smazání delimu, který zůstane po smazání posledního sloupce na řádku
                        --i;
                }
            }
            prvniPruchod=false;
        }
        if(isDelim(string[i],delim)){
            ++pocetSloupcu;
            prvniPruchod=true;
        }
    }
    return 0;
}

/**
 * Smaže sloupce v intervalu <zacatek,konec> z řetězce 
 * 
 * @param {char *} string - Řetězec, ze kterého se mají mazat sloupce
 * @param {int} zacatek - Číslo sloupce, od kterého se má začít mazat
 * @param {int} konec - Číslo sloupce, který bude smazán jako poslední
 * @return {int} - Vrací hodnotu určující, zda se příkaz povedl vykonat
*/
int dcols(char *string,int zacatek, int konec,char *delim) {
    if(!validateColSelect(zacatek)) return 1;
    if(!validateInterval(zacatek,konec)) return 1;
    else {
        while(konec>=zacatek)
        {
            dcol(konec,string,delim);
            --konec;
        }
    }
    return 0;
}

/* Vypíše prázný řádek
 * Použití: Po provedení všech příkazů se pouze připíší tyto řádky (může jich být více) */ 
void irow() {
    printf("\n");
}

/* Vypisuje příkazy arow, umožňuje víc arow příkazů v sekvenci
 * Použití: Po vypsání posledního řádku se zavolá tato funkce */
void printArows(int ArowCommand[]) {
    while(ArowCommand[0]!=0)
    {
        irow();
        arrMoveAllToLeft(ArowCommand);
    }
}

// Smaže text řetězce (řádku)
void drow(char *line) {
    line[0]='\0';
}

/************************************************************ Obsluhy příkazů - Povinné příkazy *************************************************************************/

/* obsluha příkazu cset C STR: do buňky ve sloupci C bude nastaven řetězec STR. */
void cset(int sloupec,char *nastav,char *line,char *delim) {
        int aktualniSloupec=1;
        for(int i=0;line[i]!='\0'&&line[i]!='\n';++i)
        {
            if(aktualniSloupec==sloupec){
                smazTextSloupce(line,i,delim);
                nastavString(line,LINE_MAX_LENGTH,i,nastav);
                return;//Sloupec smazán, lze ukončit
            }
            if(isDelim(line[i],delim)) ++aktualniSloupec;
        }
}

/* obsluha příkazu tolower C: řetězec ve sloupci C bude převeden na malá písmena. */
int toLower(char *line,int sloupec,char *delim) {
    int aktualniSloupec=1;
    for(int i=0;line[i]!='\0'&&line[i]!='\n';++i)
    {
        if(aktualniSloupec==sloupec) charToLower(i,line);
        if(isDelim(line[i],delim)){
            if(aktualniSloupec==sloupec)return 0; //Požadovaný sloupec to již našlo, může se ukončit
            ++aktualniSloupec;
        } 
    }
    return 0;
}

/* obsluha příkazu toupper C: řetězec ve sloupce C bude převeden na velká písmena. */
int toUpper(char *string,int sloupec,char *delim) {
    int aktualniSloupec=1;
    for(int i=0;string[i]!='\0'&&string[i]!='\n';++i)
    {
        if(aktualniSloupec==sloupec)charToUpper(i,string);
        if(isDelim(string[i],delim)){
             if(aktualniSloupec==sloupec) return 0;//Sloupec již byl upraven, nemusím pokračovat
             ++aktualniSloupec;
        }
    }
    return 0;
}

/* Obsluha příkazu round C: ve sloupci C zaokrouhlí číslo na celé číslo. */
int roundCol(char *line,int sloupec,char *delim) {
    int aktualniSloupec=1;
    for(int i=0;line[i]!='\0'&&line[i]!='\n';++i)
    {
        if(aktualniSloupec==sloupec){
            int velikostPole=getSizeOfColString(line,delim,i);
            char substringArr[velikostPole];//Vytvoření adresy pro char *substring
            char *substring=getSubstring(i,line,substringArr,velikostPole);
            bool jeCislo=kontrolaIsCislo(substring);
            if(jeCislo){
                int rounded=my_floor(substring);
                smazTextSloupce(line,i,delim);
                char array[MAX_LENGTH_INT_STRING];//vytvoření adresy pro char * z čísla
                char *nastav=intToString(rounded,array);
                nastavString(line,LINE_MAX_LENGTH,i,nastav);
            }
            else {
                printError(ERR_IN_COL_IS_NOT_NUM);
                return 1;
            }
            break;
        }
        if(isDelim(line[i],delim))++aktualniSloupec;
    }
    return 0;
}

/* Obsluha příkazu int C: odstraní desetinnou část čísla ve sloupci C. */
int intCol(char *line,int sloupec,char *delim) {
    int aktualniSloupec=1;
    for(int i=0;line[i]!='\0'&&line[i]!='\n';++i)
    {
        if(aktualniSloupec==sloupec){
            int velikostPole=getSizeOfColString(line,delim,i);
            char substringArr[velikostPole];//Vytvoření adresy pro char *substring
            char *substring=getSubstring(i,line,substringArr,velikostPole);
            bool jeCislo=kontrolaIsCislo(substring);
            if(jeCislo){
                smazTextSloupce(line,i,delim);
                float cislof=atof(substring);//Nejprve na float, protože atoi neumí spracovat čísla typu 2e2
                int intCislo=(int) cislof;
                const int INT_LENGTH_AS_STRING=11;
                char array[INT_LENGTH_AS_STRING];//vytvoření adresy pro char * z čísla
                char *nastav=intToString(intCislo,array);
                nastavString(line,LINE_MAX_LENGTH,i,nastav);
            }
            else {
                printError(ERR_IN_COL_IS_NOT_NUM);
                return 1;
            }
            break;
        }
        if(isDelim(line[i],delim))++aktualniSloupec;
    }
    return 0;
}

/* Obsluha příkazu copy N M: přepíše obsah buněk ve sloupci M hodnotami ze sloupce N. */
int copy(char *line, int colCopy,int colPaste,char *delim) {
    if(!validateColSelect(colCopy)||!validateColSelect(colPaste)) return 1;
    if(colCopy==colPaste)return 0;
    int poziceColCopy=getPosOfCol(line,colCopy,delim);
    int poziceColPaste=getPosOfCol(line,colPaste,delim);
    if(poziceColCopy==-1||poziceColPaste==-1)return 1;

    int velikostPole=getSizeOfColString(line,delim,poziceColCopy);
    char substringArr[velikostPole];
    char *substring=getSubstring(poziceColCopy,line,substringArr,velikostPole);

    smazTextSloupce(line,poziceColPaste,delim);
    nastavString(line,LINE_MAX_LENGTH,poziceColPaste,substring);
    return 0;
}

/* Obsluha příkazu swap N M: zamění hodnoty buněk ve sloupcích N a M. */
int swap(char *line,int colCopy,int colPaste,char *delim) {
    if(!validateColSelect(colCopy)||!validateColSelect(colPaste)) return 1;
    if(colCopy==colPaste)return 0;
    int poziceColPaste=getPosOfCol(line,colPaste,delim);
    int poziceColCopy=getPosOfCol(line,colCopy,delim);
    if(poziceColCopy==-1||poziceColPaste==-1)return 1;

    int arrLengthCopy=getSizeOfColString(line,delim,poziceColCopy);
    char colCopySubstringArr[arrLengthCopy];
    getSubstring(poziceColCopy,line,colCopySubstringArr,arrLengthCopy);

    int arrLengthPaste=getSizeOfColString(line,delim,poziceColPaste);
    char colPasteSubstringArr[arrLengthPaste];
    getSubstring(poziceColPaste,line,colPasteSubstringArr,arrLengthPaste);

    if(poziceColCopy>poziceColPaste){//Prvně provedu větší pozici sloupce, abych si mazáním neovlivnil tu menší
        removeCol(line,delim,poziceColCopy);
        insertCol(line,colPasteSubstringArr,poziceColCopy,delim);
        removeCol(line,delim,poziceColPaste);
        insertCol(line,colCopySubstringArr,poziceColPaste,delim);
    }
    else {
        removeCol(line,delim,poziceColPaste);
        insertCol(line,colCopySubstringArr,poziceColPaste,delim);
        removeCol(line,delim,poziceColCopy);
        insertCol(line,colPasteSubstringArr,poziceColCopy,delim);
    }
    return 0;
}

/* Obsluha příkazu move N M: přesune sloupec N před sloupec M. */
int move(char *line,int colCopy,int colPaste,char *delim) {
    if(!validateColSelect(colCopy)||!validateColSelect(colPaste)) return 1;
    if(colCopy==colPaste)return 0;
    int poziceColCopy=getPosOfCol(line,colCopy,delim);
    int poziceColPaste=getPosOfCol(line,colPaste,delim);
    if(poziceColCopy==-1||poziceColPaste==-1)return 1;

    int velikostPole=getSizeOfColString(line,delim,poziceColCopy);
    char substringArr[velikostPole];
    char *substring=getSubstring(poziceColCopy,line,substringArr,velikostPole);

    int smazano=removeCol(line,delim,poziceColCopy);
    if(colCopy<colPaste) poziceColPaste-=smazano;
    insertCol(line,substring,poziceColPaste,delim);
    return 0;
}

/***************************************************************************** Obsluhy příkazů - Volitelné příkazy ***********************************************************/

/* Obsluha příkazu csum C N M - do buňky ve sloupci C bude uloženo číslo reprezentující součet hodnot buněk na stejném řádku
   ve sloupcích N až M včetně (N <= M, C nesmí patřit do intervalu <N;M>). */
int csum(char *line,int colSaveTo,int colStart,int colEnd,char *delim) {
    if(!validateInterval(colStart,colEnd)) return 1;
    if(!validateColSelect(colSaveTo)) return 1;
    if(validateNumInInterval(colStart,colEnd,colSaveTo)) return 1;

    int poziceSaveTo=getPosOfCol(line,colSaveTo,delim);
    int poziceColStart=getPosOfCol(line,colStart,delim);
    int poziceColEnd=getPosOfCol(line,colEnd,delim);
    if(poziceSaveTo>=0&&poziceColEnd>=0){
        poziceColEnd=findNextDelim(line,delim,poziceColEnd+1);
        float soucet=0;
        for(int i=poziceColStart-1;i<poziceColEnd;++i)
        {
            double cislo=readNumFromCol(line,&i,delim);
            if(!validateStrPos(i)) return 1;
            soucet+=cislo;
        }
        char cisloStr[MAX_LENGTH_FLOAT_STRING];
        snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",soucet);
        smazTextSloupce(line,poziceSaveTo,delim);
        nastavString(line,LINE_MAX_LENGTH,poziceSaveTo,cisloStr);
    }
    else return 1;
    return 0;
}

/* Obsluha příkazu cavg C N M - obdobně jako csum, avšak výsledná hodnota představuje aritmetický průměr hodnot. */
int cavg(char *line,int colSaveTo,int colStart,int colEnd,char *delim) {
    if(!validateInterval(colStart,colEnd)) return 1;
    if(!validateColSelect(colSaveTo)) return 1;
    if(validateNumInInterval(colStart,colEnd,colSaveTo)) return 1;

    int poziceSaveTo=getPosOfCol(line,colSaveTo,delim);
    int poziceColStart=getPosOfCol(line,colStart,delim);
    int poziceColEnd=getPosOfCol(line,colEnd,delim);
    if(poziceSaveTo>=0&&poziceColEnd>=0){
        poziceColEnd=findNextDelim(line,delim,poziceColEnd+1);
        float soucet=0;
        int pocet=0;
        for(int i=poziceColStart-1;i<poziceColEnd;++i)
        {
            double cislo=readNumFromCol(line,&i,delim);
            if(!validateStrPos(i)) return 1;
            soucet+=cislo;
            ++pocet;
        }
        float vysledek=soucet/(pocet);
        char cisloStr[MAX_LENGTH_FLOAT_STRING];
        snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",vysledek);
        smazTextSloupce(line,poziceSaveTo,delim);
        nastavString(line,LINE_MAX_LENGTH,poziceSaveTo,cisloStr);
    }
    else return 1;
    return 0;
}

/* Obsluha příkazu cmin C N M - obdobně jako csum, avšak výsledná hodnota představuje nejmenší nalezenou hodnotu. */
int cmin(char *line,int colSaveTo,int colStart,int colEnd,char *delim) {
    if(!validateInterval(colStart,colEnd)) return 1;
    if(!validateColSelect(colSaveTo)) return 1;
    if(validateNumInInterval(colStart,colEnd,colSaveTo)) return 1;

    int poziceSaveTo=getPosOfCol(line,colSaveTo,delim);
    int poziceColStart=getPosOfCol(line,colStart,delim);
    int poziceColEnd=getPosOfCol(line,colEnd,delim);
    if(poziceSaveTo>=0&&poziceColEnd>=0){
        poziceColEnd=findNextDelim(line,delim,poziceColEnd+1);
        float min=INT_MAX;
        for(int i=poziceColStart-1;i<poziceColEnd;++i)
        {
            double cislo=readNumFromCol(line,&i,delim);
            if(!validateStrPos(i)) return 1;
            if(cislo<min)min=cislo;
        }
        char cisloStr[MAX_LENGTH_FLOAT_STRING];
        snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",min);
        smazTextSloupce(line,poziceSaveTo,delim);
        nastavString(line,LINE_MAX_LENGTH,poziceSaveTo,cisloStr);
    }
    else return 1;
    return 0;
}

/* Obsluha příkazu cmax C N M - obdobně jako cmin, jedná se však o maximální nalezenou hodnotu. */
int cmax(char *line,int colSaveTo,int colStart,int colEnd,char *delim) {
    if(!validateInterval(colStart,colEnd)) return 1;
    if(!validateColSelect(colSaveTo)) return 1;
    if(validateNumInInterval(colStart,colEnd,colSaveTo)) return 1;

    int poziceSaveTo=getPosOfCol(line,colSaveTo,delim);
    int poziceColStart=getPosOfCol(line,colStart,delim);
    int poziceColEnd=getPosOfCol(line,colEnd,delim);
    if(poziceSaveTo>=0&&poziceColEnd>=0){
        poziceColEnd=findNextDelim(line,delim,poziceColEnd+1);
        float max=INT_MIN;
        for(int i=poziceColStart-1;i<poziceColEnd;++i)
        {
            double cislo=readNumFromCol(line,&i,delim);
            if(!validateStrPos(i)) return 1;
            if(cislo>max)max=cislo;
        }
        char cisloStr[MAX_LENGTH_FLOAT_STRING];
        snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",max);
        smazTextSloupce(line,poziceSaveTo,delim);
        nastavString(line,LINE_MAX_LENGTH,poziceSaveTo,cisloStr);
    }
    else return 1;
    return 0;
}

/* Obsluha příkazu ccount C N M - obdobně jako csum, avšak výsledná hodnota představuje počet neprázdnných hodnot daných buněk. */
int ccount(char *line,int colSaveTo,int colStart,int colEnd,char *delim) {
    if(!validateInterval(colStart,colEnd)) return 1;
    if(!validateColSelect(colSaveTo)) return 1;
    if(validateNumInInterval(colStart,colEnd,colSaveTo)) return 1;

    int poziceSaveTo=getPosOfCol(line,colSaveTo,delim);
    int poziceColStart=getPosOfCol(line,colStart,delim);
    int poziceColEnd=getPosOfCol(line,colEnd,delim);
    if(poziceSaveTo>=0&&poziceColEnd>=0){
        poziceColEnd=findNextDelim(line,delim,poziceColEnd+1);
        int pocet=0;
        for(int i=poziceColStart-1;i<poziceColEnd;++i)
        {
            int velikostPole=getSizeOfColString(line,delim,i+1);
            char substringArr[velikostPole];
            char *substring=getSubstring(i+1,line,substringArr,velikostPole);
            if(substring[0]!='\0')++pocet;
            i+=velikostPole;
        }
        char cisloStr[MAX_LENGTH_FLOAT_STRING];
        snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%d",pocet);
        smazTextSloupce(line,poziceSaveTo,delim);
        nastavString(line,LINE_MAX_LENGTH,poziceSaveTo,cisloStr);
    }
    else return 1;
    return 0;
}

/* Obsluha příkazu cseq N M B - do buněk ve sloupcích N až M včetně vloží postupně rostoucí čísla (o jedničku) počínaje hodnotou B. */
int cseq(char *line,int colStart,int colEnd,double startNum,char *delim) {
    if(!validateInterval(colStart,colEnd)) return 1;
    int poziceColStart=getPosOfCol(line,colStart,delim);
    int poziceColEnd=getPosOfCol(line,colEnd,delim);
    if(poziceColEnd>=0){
        poziceColEnd=findNextDelim(line,delim,poziceColEnd+1);
        for(int i=poziceColStart;i<poziceColEnd;++i)
        {
            char cisloStr[MAX_LENGTH_FLOAT_STRING];
            snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",startNum);
            int smazano=smazTextSloupce(line,i,delim);
            poziceColEnd-=(smazano);

            nastavString(line,LINE_MAX_LENGTH,i,cisloStr);
            i+=strlen(cisloStr);
            poziceColEnd+=strlen(cisloStr);
            ++startNum;
        }
    }
    else return 1;
    return 0;
}

/* Obsluha příkazu rseq C N M B - ve sloupci C do buněk každého řádku od řádku N po řádek M včetně vloží rostoucí čísla počínaje hodnotou B.
   Číslo M může být nahrazeno pomlčkou. V takovém případě se tím myslí poslední řádek souboru. */
int rseq(char *line,int colSelect,double *setNum,char *delim) {
    int posCol=getPosOfCol(line,colSelect,delim);
    if(posCol<0)return 1;
    smazTextSloupce(line,posCol,delim);

    char cisloStr[MAX_LENGTH_FLOAT_STRING];
    snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",*setNum);
    nastavString(line,LINE_MAX_LENGTH,posCol,cisloStr);
    (*setNum)++;
    return 0;
}

/* Obsluha příkazu rsum C N M - do buňky ve sloupci C na řádku M+1 vloží součet hodnot buněk ve sloupci C na řádcích N až M včetně. */
int rsum(char *line,int colSelect,double *soucet,char *delim) {
    int posCol=getPosOfCol(line,colSelect,delim);
    if(posCol<0)return 1;
    double cislo=readNumFromCol(line,&posCol,delim);
    if(!validateStrPos(posCol)) return 1;
    *soucet+=cislo;
    return 0;
}

/* Obsluha příkazu ravg C N M - obdobně jako rsum, avšak výsledná hodnota představuje aritmetický průměr. */
int rmin(char *line,int colSelect,double *min,char *delim) {
    int posCol=getPosOfCol(line,colSelect,delim);
    if(posCol<0)return 1;
    double cislo=readNumFromCol(line,&posCol,delim);
    if(!validateStrPos(posCol)) return 1;
    if(cislo<*min)*min=cislo;
    return 0;
}

/* Obsluha příkazu rmin C N M - obdobně jako rsum, avšak výsledná hodnota představuje nejmenší hodnotu. */
int rmax(char *line,int colSelect,double *max,char *delim) {
    int posCol=getPosOfCol(line,colSelect,delim);
    if(posCol<0)return 1;
    double cislo=readNumFromCol(line,&posCol,delim);
    if(!validateStrPos(posCol)) return 1;
    if(cislo>*max)*max=cislo;
    return 0;
}

/* Obsluha příkazu rmax C N M - obdobně jako rsum, avšak výsledná hodnota představuje největší hodnotu. */
int rcount(char *line,int colSelect,SharedVariables *sharedVariable,char *delim) {
    int posCol=getPosOfCol(line,colSelect,delim);
    if(posCol<0)return 1;
    if(line[posCol]!=':'&&line[posCol]!='\n')sharedVariable->vysledek++;
    return 0;
}

/* Obsluha příkazu rcount C N M - obdobně jako rsum, avšak výsledná hodnota představuje počet neprázdnných hodnot daných buněk. */
int ravg(char *line,int colSelect,SharedVariables *sharedVariable,char *delim) {
    int posCol=getPosOfCol(line,colSelect,delim);
    if(posCol<0)return 1;
    double cislo=readNumFromCol(line,&posCol,delim);
    if(!validateStrPos(posCol)) return 1;
    sharedVariable->vysledek+=cislo;
    sharedVariable->pocet++;
    return 0;
}

/** 
 * Vypíše výsledek z provádění příkazů rseq - rmax do řádku M+1 do zvoleného sloupce
 * @param {SharedVariables} sharedVariable - Struktura, kterou sdílí všechny rxxx příkazy, ukládá počet a patřičný výsledek
 * @param {bool} avg - Určuje, zda se má spočítat průměr, nebo stačí vypsat výsledek
 */
int writeRowResult(char *line,int colSelect,SharedVariables *sharedVariable,bool avg,char *delim) {
    int posCol=getPosOfCol(line,colSelect,delim);
    if(posCol<0)return 1;
    smazTextSloupce(line,posCol,delim);
    char cisloStr[MAX_LENGTH_FLOAT_STRING];

    if(!avg){
        snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",sharedVariable->vysledek);
        nastavString(line,LINE_MAX_LENGTH,posCol,cisloStr);
    }else if(sharedVariable->pocet>0){
        double prumer=sharedVariable->vysledek/sharedVariable->pocet;
        snprintf(cisloStr,MAX_LENGTH_FLOAT_STRING,"%g",prumer);
        nastavString(line,LINE_MAX_LENGTH,posCol,cisloStr);
    }
    return 0;
}

/******************************************************************************* Příkazy pro zpracování argumentů ************************************************************/

/* Načte argumenty potřebné pro příkazy, které pracují s intervalem a selectem sloupce/řádku */
int loadIntervalArgs(ArgsOfIntervalCommands *args,int argc,char *argv[],int *posToRead)
{
    args->colSelect=NactiDalsiArgumentToInt(argc,argv,posToRead,false);
    args->intervalStart=NactiDalsiArgumentToInt(argc,argv,posToRead,args->colSelect);
    if(!validateRowNum(args->intervalStart)) return 1;
    args->intervalEnd=NactiDalsiArgumentToInt(argc,argv,posToRead,args->intervalStart); 
    if(args->intervalEnd==-1) return 1;
    return 0;
}

//Prochází argumenty příkazové řádky a hledá příkaz pro zpracování dat, první z nich provede (Pouze první)
int findAndExecCommand(char *line,int argc,char *argv[],char *delim) {
    int err=0;
    ArgsOfIntervalCommands args;
    for(int i=1;i<argc;++i)
    {
        if(strcmp(argv[i],"cset")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
            char *nastav=NactiDalsiArgument(argc,argv,&i,sloupec);
            if(nastav==NULL) return 1;
            cset(sloupec,nastav,line,delim);
            return 0;
        }
        else if(strcmp(argv[i],"tolower")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
            if(sloupec==-1)return 1;
            toLower(line,sloupec,delim);
            return 0;
        }
        else if(strcmp(argv[i],"toupper")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
            if(sloupec==-1) return 1;
            toUpper(line,sloupec,delim);
            return 0;
        }
        else if(strcmp(argv[i],"round")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
            if(sloupec==-1)return 1;
            err=roundCol(line,sloupec,delim);
            return err;
        }
        else if(strcmp(argv[i],"int")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
            if(sloupec==-1) return 1;
            err=intCol(line,sloupec,delim);
            return err;
        }
        else if(strcmp(argv[i],"copy")==0){
            int colCopy=NactiDalsiArgumentToInt(argc,argv,&i,false);
            int colPaste=NactiDalsiArgumentToInt(argc,argv,&i,colCopy);
            if(colPaste==-1) return 1;
            err=copy(line,colCopy,colPaste,delim);
            return err;
        }
        else if(strcmp(argv[i],"swap")==0){
            int colCopy=NactiDalsiArgumentToInt(argc,argv,&i,false);
            int colPaste=NactiDalsiArgumentToInt(argc,argv,&i,colCopy);
            if(colPaste==-1) return 1;
            err=swap(line,colCopy,colPaste,delim);
            return err;
        }
        else if(strcmp(argv[i],"move")==0){
            int colCopy=NactiDalsiArgumentToInt(argc,argv,&i,false);
            int colPaste=NactiDalsiArgumentToInt(argc,argv,&i,colCopy);
            if(colPaste==-1) return 1;
            err=move(line,colCopy,colPaste,delim);
            return err;
        }
        else if(strcmp(argv[i],"csum")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            err=csum(line, args.colSelect, args.intervalStart, args.intervalEnd,delim);
            return err;
        }
        else if(strcmp(argv[i],"cavg")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            err=cavg(line, args.colSelect, args.intervalStart, args.intervalEnd,delim);
            return err;
        }       
        else if(strcmp(argv[i],"cmin")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            err=cmin(line,args.colSelect, args.intervalStart, args.intervalEnd,delim);
            return err;
        }
        else if(strcmp(argv[i],"cmax")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            err=cmax(line, args.colSelect, args.intervalStart, args.intervalEnd,delim);
            return err;
        }
        else if(strcmp(argv[i],"ccount")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            err=ccount(line, args.colSelect, args.intervalStart, args.intervalEnd,delim);
            return err;        
        }
        else if(strcmp(argv[i],"cseq")==0){
            int colStart=NactiDalsiArgumentToInt(argc,argv,&i,false);
            int colEnd=NactiDalsiArgumentToInt(argc,argv,&i,colStart);
            double startNum=NactiDalsiArgumentToDouble(argc,argv,&i,colEnd);
            if(startNum==-1)return 1;
            err=cseq(line,colStart,colEnd,startNum,delim);
            return err;
        }
    }
    return 0;
}

/**
 * Načtení všech příkazů pro úpravu řádků tabulky (Aby se nemusely ve zpracování řádků procházet všechny argumenty opakovaně) 
 * 
 * @param {int *} argc - Počet argumentů z terminálu - toto číslo se bude snižovat po načtení příkazu
 * @param {char *[]} argv - Argumenty z terminálu
 * @param {int [][]} rowCommands - Dvourozměrné pole pro načítání příkazů pro úpravu velikosti tabulky
 * @return {int} - Vrací 0, pokud vše proběhne bez chyby, 1 pokud někde nastane chyba                       
*/
int loadRowCommands(int *argc,char *argv[],int rowCommands[NUMBER_ROW_COMMANDS][*argc]) {
    int posIROW = 0,posAROW = 0,posDROW = 0,posDROWS = 0;
    for(int i=1;i<*argc;++i)
    {
        if(strcmp(argv[i],"irow")==0){
            int radek=NactiDalsiArgumentToInt(*argc,argv,&i,false);
            if(radek==-1)return 1;
            rowCommands[IROW][posIROW]=radek;
            ++posIROW;
            presunVPoli(argc,argv,&i);   
            presunVPoli(argc,argv,&i);     
        }
        else if(strcmp(argv[i],"arow")==0){
            rowCommands[AROW][posAROW]=1;
            ++posAROW;
            presunVPoli(argc,argv,&i);
        }
        else if(strcmp(argv[i],"drow")==0){
            int radek=NactiDalsiArgumentToInt(*argc,argv,&i,false);
            if(radek==-1)return 1;
            rowCommands[DROW][posDROW]=radek;
            ++posDROW;
            presunVPoli(argc,argv,&i);
            presunVPoli(argc,argv,&i);
        }
        else if(strcmp(argv[i],"drows")==0){
            int zacatek=NactiDalsiArgumentToInt(*argc,argv,&i,false);
            int konec=NactiDalsiArgumentToInt(*argc,argv,&i,zacatek);
            if(konec==-1)return 1;
            rowCommands[DROWS][posDROWS]=zacatek;
            ++posDROWS;
            rowCommands[DROWS][posDROWS]=konec;
            ++posDROWS;
            presunVPoli(argc,argv,&i);
            presunVPoli(argc,argv,&i);
            presunVPoli(argc,argv,&i);
        }
    }
    rowCommands[IROW][posIROW]=0;
    rowCommands[AROW][posAROW]=0;
    rowCommands[DROW][posDROW]=0;
    rowCommands[DROWS][posDROWS]=0;
    sortArray(rowCommands[IROW],posIROW);
    sortArray(rowCommands[AROW],posAROW);
    sortArray(rowCommands[DROW],posDROW);
    sortArray(rowCommands[DROWS],posDROWS);
    return 0;
}

/**
 * Najde a provede všechny příkazy pro úpravu velikosti tabulky - počet řádků
 * 
 * @param {char *} line - Řetěřec, pro který se teoreticky může projevit změna počtu řádků (Smazání, přidání před tento řádek...)
 * @param {int} puvArgc - Původní počet argumentů z příkazové řádky, pro určení maximální velikosti dvourozměrného pole 
 * @param {int} numLine - Číslo aktuálně zpracovávaného řádku
 * @param {int[][]} rowCommands - Dvourozměrné pole obsahující jednotlivé příkazy pro úpravu velikosti tabulky - počtu řádků
 * @return {int} - vrací 0, pokud vše proběhne bez chyby, 1 pokud nastane nějaká chyba
 * 
 * Poznámka: numLine se pořád bere jako původní numLine i pokud se nějaký řádek smaže/přidá,
 *            aby uživatel nemusel přemýšlet, když smaže řádek x, kolikátý má nyní upravit 
 */
int processLineRow(char *line,int puvArgc,int numLine,int rowCommands[NUMBER_ROW_COMMANDS][puvArgc]) {
    if(rowCommands[IROW][0]==numLine){
        irow();
        arrMoveAllToLeft(rowCommands[IROW]);
    }
    else if(rowCommands[DROW][0]==numLine){
        drow(line);
        arrMoveAllToLeft(rowCommands[DROW]);
    }
    else if(rowCommands[DROWS][0]==numLine){
        drow(line);
        rowCommands[DROWS][0]++;//První číslo v intervalu se postupně zvyšuje
        if(rowCommands[DROWS][0]>rowCommands[DROWS][1]){
            int lastNum=(rowCommands[DROWS][0]-1); 
            arrMoveAllToLeft(rowCommands[DROWS]);
            arrMoveAllToLeft(rowCommands[DROWS]);
            //Pokud bylo ve více argumentech zadáno stejné číslo, aby se toto číslo promazalo, jinak to už nikdy neprojede pro vyšší čísla
            while(rowCommands[DROWS][0]==lastNum) //Smazání z pole všech čísel, které jsou stejné jako poslední číslo
            {
                arrMoveAllToLeft(rowCommands[DROWS]);
            }
        }
    }
    return 0;
}

/* Získá počet příkazů úpravujících počet sloupců a které navíc potřebují ke své práci číslo */
int getNumOfColCommands(char *line,char *delim,int argc,char *argv[])
{
    int pocetCommands=0;
    for(int i=1;i<argc;++i)
    {
        if(strcmp(argv[i],"icol")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
            if(sloupec==-1) return -1;
            ++pocetCommands;
        }
        else if(strcmp(argv[i],"acol")==0){
            acol(line,delim);
        }
        else if(strcmp(argv[i],"dcol")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
            if(sloupec==-1) return -1;
            ++pocetCommands;
        }
        else if(strcmp(argv[i],"dcols")==0){
            int zacatek=NactiDalsiArgumentToInt(argc,argv,&i,false);
            int konec=NactiDalsiArgumentToInt(argc,argv,&i,zacatek);
            if(konec==-1)return -1;
            ++pocetCommands;
        }
    }
    return pocetCommands;
}

/* Provede příkaz pro úpravu počtu sloupců na pozici zjištěného maxima (maximum bez již vykonaných sloupců větší než toto max) */
void processColCommands(char *line,char *delim,int argc,char *argv[], int posMax){
        if(strcmp(argv[posMax],"icol")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&posMax,false);
            icol(sloupec,line,delim);
        }
        else if(strcmp(argv[posMax],"dcol")==0){
            int sloupec=NactiDalsiArgumentToInt(argc,argv,&posMax,false);
            dcol(sloupec,line,delim);
        }
        else if(strcmp(argv[posMax],"dcols")==0){
            int zacatek=NactiDalsiArgumentToInt(argc,argv,&posMax,false);
            int konec=NactiDalsiArgumentToInt(argc,argv,&posMax,false);
            dcols(line,zacatek,konec,delim);
        }
}

/**
 * Najde a provede všechny příkazy pro úpravu velikosti tabulky - počet sloupců, provádí příkazy od největšího čísla sloupce po nejmenší, aby se neovlivnovali navzájem
 * 
 * @param {char *} line - Řetěřec, nad kterým se budou přidávat / ubírat sloupce
 * @param {int} argc - Číslo reprezentující počet argumentů (Velikost pole řetězců)
 * @param {char *[]} argv - Pole řetězců obsahující příkazy, včetně těch pro úpravu velikosti tabulky - počtu sloupců
 * @return {int} - vrací 0, pokud vše proběhne bez chyby, 1 pokud nastane nějaká chyba
*/
int processLineCols(char *line,int argc,char *argv[],char *delim) {
    int pocetCommands=getNumOfColCommands(line,delim,argc,argv);
    if(pocetCommands==-1)return 1;

    int predchoziMax=RAND_MAX;
    while(pocetCommands>0) //Budu provádět dokud jsem neprovedl všechny příkazy a od největšího čísla sloupce po nejmenší!!
    {
        int max=-RAND_MAX;
        int posMax=0;
        for(int i=1;i<argc;++i)
        {
            if(strcmp(argv[i],"icol")==0){
                int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
                if(sloupec>max&&sloupec<predchoziMax){
                    max = sloupec;
                    posMax=i-1;
                }
            }
            else if(strcmp(argv[i],"dcol")==0){
                int sloupec=NactiDalsiArgumentToInt(argc,argv,&i,false);
                if(sloupec>max&&sloupec<predchoziMax){
                    max = sloupec;
                    posMax=i-1;
                }
            }
            else if(strcmp(argv[i],"dcols")==0){
                int zacatek=NactiDalsiArgumentToInt(argc,argv,&i,false);
                if(zacatek>max && zacatek < predchoziMax){
                    max = zacatek;
                    posMax=i-1;
                }
            }
        }
        processColCommands(line,delim,argc,argv,posMax);
        predchoziMax=max;
        --pocetCommands;
    }
    return 0;
}


/**
 * Prochází argumenty příkazové řádky a hledá příkazy typu rxxx, první z nich provede (Pouze první)
 * @param {int} lineCount - Číslo označující číslo řádku v celém souboru
 * @param {SharedVariables} sharedVariables - Proměnná pro ukládání průběžných výsledků příkazů rxxx 
*/ 
int findAndExecRowsCommand(char *line,int argc,char *argv[],int lineCount,SharedVariables *sharedVariables,char *delim) {
    int err=0;
    ArgsOfIntervalCommands args;
    for(int i=1;i<argc;++i)
    {
        if(strcmp(argv[i],"rseq")==0){
            loadIntervalArgs(&args,argc,argv,&i);
            int startNum=NactiDalsiArgumentToInt(argc,argv,&i,args.intervalEnd);
            if(startNum==-1)return 1;
            if(lineCount>=args.intervalStart && lineCount<=args.intervalEnd ){
                sharedVarSetDefaultValue(sharedVariables,startNum);
                err=rseq(line,args.colSelect,&(sharedVariables->vysledek),delim);
            }
            return err;
        }
        else if(strcmp(argv[i],"rsum")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            if(lineCount >= args.intervalStart && lineCount <= args.intervalEnd ) err = rsum(line, args.colSelect, &(sharedVariables->vysledek), delim);
            else if( lineCount == args.intervalEnd+1 ) err = writeRowResult(line, args.colSelect, sharedVariables, false, delim);
            return err;
        }
        else if(strcmp(argv[i],"ravg")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            if(lineCount >= args.intervalStart && lineCount <= args.intervalEnd) err=ravg(line,args.colSelect,sharedVariables, delim);
            if(lineCount == args.intervalEnd+1 ) err = writeRowResult(line, args.colSelect, sharedVariables, true, delim);
            return err;
        }
        else if(strcmp(argv[i],"rmin")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            if(lineCount >= args.intervalStart && lineCount <= args.intervalEnd){
                sharedVarSetDefaultValue(sharedVariables,RAND_MAX);
                err = rmin(line, args.colSelect, &(sharedVariables->vysledek), delim);
            }
            if(lineCount == args.intervalEnd+1) err = writeRowResult(line, args.colSelect, sharedVariables, false, delim);
            return err;
        }
        else if(strcmp(argv[i],"rmax")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            if(lineCount >= args.intervalStart && lineCount <= args.intervalEnd){
                sharedVarSetDefaultValue(sharedVariables,-RAND_MAX);
                err=rmax(line, args.colSelect, &(sharedVariables->vysledek), delim);
            }
            if(lineCount == args.intervalEnd+1) err = writeRowResult(line, args.colSelect, sharedVariables, false, delim);
            return err;
        }
        else if(strcmp(argv[i],"rcount")==0){
            if(loadIntervalArgs(&args,argc,argv,&i)) return 1;
            if(lineCount >= args.intervalStart && lineCount <= args.intervalEnd) err = rcount(line, args.colSelect, sharedVariables, delim);
            if(lineCount == args.intervalEnd+1) err=writeRowResult(line, args.colSelect ,sharedVariables,false, delim);
            return err;
        }
    }
    return 0;
}

/* Zkontroluje, zda řádek vyhovuje podmínce z příkazu: 
            rows N M - procesor bude zpracovávat pouze řádky N až M včetně (N <= M). N=1 znamená zpracování od prvního řádku.
            Pokud je místo čísla M zadán znak - (pomlčka), ta reprezentuje poslední řádek vstupního souboru. Pokud je pomlčka také místo sloupce N, 
            myslí se tím výběr pouze posledního řádku. Pokud není tento příkaz zadán, uvažuje se implicitně o všech řádcích.  
*/
int lineSelectionsNumsBased(int lineCount,char *argv[],int posRowSelectCommand,bool lastLine) {
        int selectRowStart=0;
        int selectRowEnd=0;
        if(argv[posRowSelectCommand+1][0]=='-'&&argv[posRowSelectCommand+1][1]=='\0'){//argv[pozice] musí být "-\0"
            if(argv[posRowSelectCommand+2][0]=='-'&&argv[posRowSelectCommand+2][1]=='\0'){
                if(lastLine)return true;
                else return false;
            }
            else {//prvni je pomlcka, potom cislo, to je spatne
                printError(ERR_WRONG_INTERVAL);
                return -1;
            }
        }
        else selectRowStart=atoi(argv[posRowSelectCommand+1]); 

        if(argv[posRowSelectCommand+2][0]=='-'&&argv[posRowSelectCommand+2][1]=='\0')selectRowEnd=lineCount;
        else selectRowEnd=atoi(argv[posRowSelectCommand+2]);
        if(lineCount>0)
            if(!validateInterval(selectRowStart,selectRowEnd)) return -1;
        if(lineCount>=selectRowStart&&lineCount<=selectRowEnd)return true;
        else return false; 
}

/* Zkontroluje, zda řádek vyhovuje podmínce z příkazů: 
            a) beginswith C STR - procesor bude zpracovávat pouze ty řádky, jejichž obsah buňky ve sloupci C začíná řetězcem STR.
            b) contains C STR - procesor bude zpracovávat pouze ty řádky, jejichž buňky ve sloupci C obsahují řetězec STR.
*/
int lineSelectionsStringBased(char *line,int argc,char *argv[],int posRowSelectCommand,char *delim) {
        int colCheck=NactiDalsiArgumentToInt(argc,argv,&posRowSelectCommand,false);
        if(!validateColSelect(colCheck)) return 1;
        char *stringCheck=NactiDalsiArgument(argc,argv,&posRowSelectCommand,colCheck);
        int aktualniSloupec=1;
        for(int i=0;i>=0&&line[i]!='\0';)
        {
            if(aktualniSloupec==colCheck){
                int velikostPole=getSizeOfColString(line,delim,i);
                char substringArr[velikostPole];
                char *substring=getSubstring(i,line,substringArr,velikostPole);
                if(strcmp(argv[posRowSelectCommand],"beginswith")==0) return startsWithStr(substring,stringCheck);
                else return containsStr(substring,stringCheck); 
            }
            i=nextPositionOf(delim[0],line,i)+1;
            ++aktualniSloupec;
        }
        return false;
}

//Zkontroluje, zda daný řádek vyhovuje podmínce z příkazu pro selekci řádků
int checkIfRowFitCondition(char *line,int lineCount,int argc,char *argv[],int posRowSelectCommand,bool lastLine,char *delim) {
    if(!validateNumOfParams(2,posRowSelectCommand,argc)) return 1;
    if(strcmp(argv[posRowSelectCommand],"rows")==0) return lineSelectionsNumsBased(lineCount,argv,posRowSelectCommand,lastLine);
    else return lineSelectionsStringBased(line,argc,argv,posRowSelectCommand,delim);
}

//Tato funkce v sobě zabaluje celek pro úpravu dat tabulky
int processTableChanges(char *line,int lineCount,int argc,char *argv[],bool lastLine,SharedVariables *sharedVariables,char *delim) {
    int err=0;
    int posRowSelectCommand=getPosOfRowSelectCommand(argc,argv);
    if(posRowSelectCommand!=-1){
        int lineFitCondition=checkIfRowFitCondition(line,lineCount,argc,argv,posRowSelectCommand,lastLine,delim);
        if(lineFitCondition==-1) return 1;
        if(lineFitCondition){
            err=findAndExecCommand(line,argc,argv,delim);
            err=err||findAndExecRowsCommand(line,argc,argv,lineCount,sharedVariables,delim);
        }
    }
    else if(lastLine)return 0;
    else {//Proveď úpravu pro všechny řádky
        err =findAndExecCommand(line,argc,argv,delim);
        err=err||findAndExecRowsCommand(line,argc,argv,lineCount,sharedVariables,delim);
    }
    return err;
}

/** 
 * Průběžně čte všechny řádky ze vstupu a ty nechává zpracovat
 * @param {int} puvArgc - Původní velikost argc (argc bylo zmenšované), z důvodu určení velikosti pole příkazů
 * @param {int[][]} rowCommands - Pole obsahující příkazy pro změnu počtu řádků a jejich argumenty
*/

int readStdin(int argc,char *argv[],int puvArgc,int rowCommands[NUMBER_ROW_COMMANDS][puvArgc],char *delim) {
    SharedVariables sharedVariables={0,0};
    char line[LINE_MAX_LENGTH];
    char lineToPrint[LINE_MAX_LENGTH]={'\0'};
    int err=0;
    for(int i=0;(fgets(line,LINE_MAX_LENGTH, stdin)!=0);++i)
    {
        printf("Vstup: %s",line);
        int realDelkaRadku=strlen(line);
        if(realDelkaRadku==(LINE_MAX_LENGTH-1)&&line[LINE_MAX_LENGTH-1]!='\n'){
            printError(ERR_LONG_LINE);
            return 1;
        }
        printf("%s",lineToPrint);
        err=processLineCols(line,argc,argv,delim);
        processLineRow(line,puvArgc,i+1,rowCommands);
        err=err||processTableChanges(line,i+1,argc,argv,false,&sharedVariables,delim);
        replaceAllDelims(line,delim);
        copyStrToStr(line,lineToPrint); //Řádek se vypisuje až v dalším cyklu, aby lastLine nebyla vypsána 2x a aby se poznalo, kdy se jedná o lastLine
        if(err) return 1;
    }
    processTableChanges(line,0,argc,argv,true,&sharedVariables,delim);
    printf("%s",line);
    printArows(rowCommands[AROW]);
    return 0;
}


int main(int argc,char *argv[]) {
    char delim[DELIM_LENGTH];
    setDelim(delim,&argc,argv);
    if(delim[0]=='\0')return EXIT_FAILURE;

    int puvArgc=argc;
    /*Pole ve kterém je první rozměr typ příkazu pro úpravu velikosti tabulky a v druhém rozměru jsou jednotlivá čísla pracující s tímto příkazem
    * načtená z argumentů terminálu přesně tak, jak šly zasebou */
    int rowCommands[NUMBER_ROW_COMMANDS][argc];
    int loaded=loadRowCommands(&argc,argv,rowCommands);
    if(loaded) return EXIT_FAILURE;

    int success=readStdin(argc, argv,puvArgc,rowCommands,delim);
    if(success)return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
