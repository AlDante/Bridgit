//
//  main.cpp
//  Bridgit
//
//  Created by David Jenkins on 07.11.20.
//


#include <iostream>
#include <iomanip>

using namespace std;

#include "/Users/david/Documents/Projects/MacDDS/include/dll.h"
#include "hands.hpp"
#include "AnalysePlayPBN.hpp"

/* https://stackoverflow.com/questions/10290610/how-can-i-find-the-number-of-elements-in-an-array
 
 Define countof to return the number of elements of a static array

 "First, the top-level stuff

 char ( &_ArraySizeHelper( ... ))[N];  // says _ArraySizeHelper is a function that returns a reference (note the &) to a char array of N elements.

 Next, the function parameter is T (&array)[N] which is a reference to a T array of N elements.

 Finally, countof is defined as the size of the result of the function _ArraySizeHelper. Note we don’t even need to define _ArraySizeHelper(), -- a declaration is enough."
 
 */
template <typename T, size_t N>
char ( &_ArraySizeHelper( T (&array)[N] ))[N];

#define countof( array ) (sizeof( _ArraySizeHelper( array ) ))

enum DDSStrain {
    Spades = 0,
    Hearts = 1,
    Diamonds = 2,
    Clubs = 3,
    NT = 4
};

enum DDSHand {
    North = 0,
    East = 1,
    South = 2,
    West = 3
};

enum DDSVulnerability {
    None = 0,
    Both = 1,
    NSVul = 2,
    EWVul = 3
};

enum DDSSide {
    NS = 0,
    EW = 1
};

/*
 PBN: "<first>:<1st_hand> <2nd_hand> <3rd_hand> <4th_hand>". Hands are separated by spaces. So PBNExample encodes

                         S: K6
                         H: QJT976
                         D: QT7
                         C: Q6
         West:                          East:
         S: T5                          S: 432
         H: K4                          H: A
         D: 652                         D: AKJ93
         C: A98542                      C: JT73
                        South:
                         S: AQJ987
                         H: 8532
                         D: 84
                         C: K

 West to lead => South is declarer.
 */
static const std::string PBNExample = "W:T5.K4.652.A98542 K6.QJT976.QT7.Q6 432.A.AKJ93.JT73 AQJ987.8532.84.K";
// The next example is from here: http://www.rpbridge.net/7a12.htm
static const std::string PBNExample2 = "W:A8765.QT.K9.AT87 J42.AJ7632.J.632 QT3.85.Q86.KQJ54 K9.K94.AT75432.9";

static const std::string rankAsString = "23456789TJQKA";
static const std::string suitAsString = "SHDCN";
static const std::string handAsString = "NESW";

/// Convert the numeric encoding of the system DDS is compiled for to a readable string
/// @param system  numeric encoding of system; 1 = Windows, 2 = Cygwin, 3=Linux, 4=Apple
std::string systemToString(int system)
{
    switch (system) {
        case 0:
            return "Unknown";
            break;
            
        case 1:
            return "Windows";
            break;
            
        case 2:
            return "Cygwin";
            break;
            
        case 3:
            return "Linux";
            break;
            
        case 4:
            return "Apple";
            break;
            
        default:
            return "Error: Undefined system";
            break;
    }
    return "Can't happen";
}

/// Convert numeric representation of a suit into a readable string
/// @param suit numeric representation of a suit. 0=spades, 1=hearts, 2=diamonds, 3=clubs, 4=no trumps
/// @return The suit name as a one character string ("S", "H", "D", "C", "N"), or "Suit out of range" if the passed in suit is invalid
std::string suitToString(int suit)
{
    
    string suitStr = "";
    if ((0 <= suit) && (suit <= 4)) {
        suitStr = suitAsString.at(suit);
            }
    else {
        suitStr = "Suit out of range";
    }

    return suitStr;
}

/// Convert numeric representation of a hand into a readable string
/// @param hand numeric representation of a suit. 0=spades, 1=hearts, 2=diamonds, 3=clubs,
/// @return The hand name as a one character string ("N", "E", "S", "W"), or "Hand out of range" if the passed in suit is invalid
std::string handToString(int hand)
{
    
    string handStr = "";
    if ((0 <= hand) && (hand <= 3)) {
        handStr = handAsString.at(hand);
            }
    else {
        handStr = "Hand out of range";
    }

    return handStr;
}


/// Convert card rank into a readable string
/// @param rank card rank. 2=deuce, 10=ten, 11=Jack, 12=Queen, 13=King, 14=Ace,
/// @return The card rank as a one character string ("2", "3", ... ,"9", "T", "J", "Q", "K", "A"), or "Rank out of range" if the passed in card rank is invalid
std::string rankToString(int rank)
{
    /*
     Rank of the returned card. Value range 2-14.
     */
    string rankStr = "";
    
    if ((2 <= rank) && (rank <= 14)) {
        rankStr = rankAsString.at(rank-2);
    }
    else {
        rankStr = "Rank out of range";
    }
    
    return rankStr;

}

/// Convert a binary representation of the cards held in a suit to a readable string
/// @param hand hand cards encoded as follows:
///
/// A value of 16388 = 16384 + 4 is the encoding for the holding “A2” (ace and deuce).
/// The two lowest bits are always zero.
///
std::string holdingToString(unsigned int hand)
{

    
    string result = "";
    
    // Least significant two bits are zero.
    hand >>= 2;
    
    for (int i = 0; (i < 13) && (hand != 0); i++) {
        if (hand & 1)
            result = rankAsString.at(i) + result;
        hand >>= 1;
    }
    
    return result;
}

/// Main program
/// @param argc Number of arguments
/// @param argv array of argument strings
int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    int suit = 0;  
    
    struct deal dl;
    /* DealPBN
     int trump;                 // Suit encoding
     int first;                 // The hand leading to the trick.  Hand encoding
     int currentTrickSuit[3];   // Up to 3 cards may already have been played to the trick.  Suit encoding.
     int currentTrickRank[3];   // Up to 3 cards may already have been played to the trick. Value range 2-14. Set to 0 if no card has been played.
     char remainCards[80];      // Remaining cards.  PBN encoding.
     
     */
    struct dealPBN dlPBN;
    int target = 1;
    int solutions = 2;
    int mode=0;
    
    /*
     struct futureTricks
     {
     int nodes;           // Number of nodes searched by the DD sol
     
     int cards;           // Number of cards for which a result is returned.
     // May be all the cards, but equivalent ranks are omitted, so for a holding of KQ76 only the cards K and 7 would be returned,
     // and the “equals” field below would be 2048 (Q) for the king and 64 (6) for the 7.
     // For KQ765: rank[0] = 13, rank[1] = 7, equals[0] = 4096 (for the Q), equals[1] = 96 (for the 6 and 5).
     
     int suit[13];        // Suit of the each returned card.  Suit encoding
     int rank[13];        // Rank of the returned card. Value range 2-14.
     int equals[13];      // Lower-ranked equals.  Holding encoding.
     int score[13];       // -1: target not reached.
     // -2 (if mode == 0):  Only one card to be played; see SolveBoard() description.
     // Otherwise: Target of maximum number of tricks.
     
     };
     */
    struct futureTricks fut;
    int threadIndex=0;
    
    // Spades are trumps, North is declarer, from opening lead so no cards played to trick.
    dlPBN.trump = DDSStrain::Hearts;
    dlPBN.first = DDSHand::East;            // East is on lead. Overrides information in PBN.
    
    dlPBN.currentTrickSuit[0] = DDSStrain::Spades;
    dlPBN.currentTrickSuit[1] = DDSStrain::Spades;
    dlPBN.currentTrickSuit[2] = DDSStrain::Spades;
    
    dlPBN.currentTrickRank[0] = 5;
    dlPBN.currentTrickRank[1] = 2;
    dlPBN.currentTrickRank[2] = 10;
    
    strncpy(dlPBN.remainCards, PBNExample2.c_str(), sizeof(dlPBN.remainCards));
    
    SetMaxThreads(0);
    char title[] = "PBNExample";    // PrintPBNHand expects a char *; string literals in C++ are const; this avoids a type error
    PrintPBNHand(title, dlPBN.remainCards);
    
    // Calculates if leader's or opponent's side will take the trick if a given card is led.
    SolveBoardPBN(dlPBN, target, solutions, mode, &fut, threadIndex);
  
    playTracePBN DDplayPBN;
    solvedPlay solved; // int number: trick number
                       // int tricks[53]: Even entries are binary-encoded holdings; odd entries are tricks. Starting position and up to 52 cards
    
    DDplayPBN.number = 1;               // 1 card played (CK) so far in this deal
    strcpy(DDplayPBN.cards, "CK");      // Any cards already played would go here like this: "HAHKHQH7D7D8DAD9C5CAC6C3"
    
    // dlPBN contains any cards already played in the trick, as well as the trump suit and the leading hand
    // DDplayPBN contains any cards already played so far
    // Solved contains the double dummy number of tricks for declarer before play and after each card played
    int res = AnalysePlayPBN(dlPBN, DDplayPBN, &solved, threadIndex);
    if (res != RETURN_NO_FAULT)    {
        char line[80];
        
        ErrorMessage(res, line);
        std::cout << "DDS error: " << line << endl;
    }
    
    PrintPBNPlay(&DDplayPBN, &solved);
    
    struct ddTableResults tableResults;
    struct ddTableDealPBN tableDealPBN;
    strncpy(tableDealPBN.cards, PBNExample2.c_str(), sizeof(tableDealPBN.cards));
    struct parResults presp;
    int vulnerable = 0; //   /* vulnerable 0: None 1: Both 2: NS 3: EW */
    
    res = CalcDDtablePBN(tableDealPBN, &tableResults);
    if (res != RETURN_NO_FAULT)    {
        char line[80];
        
        ErrorMessage(res, line);
        std::cout << "DDS error: " << line << endl;
    }
    
    res = Par(&tableResults, &presp, vulnerable);
    if (res != RETURN_NO_FAULT)    {
        char line[80];
        
        ErrorMessage(res, line);
        std::cout << "DDS error: " << line << endl;
    }

    std::cout << "Par score    " << presp.parScore[0] << endl;
    std::cout << "Par score    " << presp.parScore[1] << endl;
    std::cout << "Par contract " << presp.parContractsString[0] << endl;
    std::cout << "Par contract " << presp.parContractsString[1] << endl;
    std::cout << endl << endl << "-----------------------------------------------------" << endl << endl;

    // TestAnalysePlayPBN();
    
    // Get some information about the Double Dummy Solver
    struct DDSInfo info;
    GetDDSInfo(&info);
    
    std::cout << "Version (major:minor:patch): " << info.major << ":"  << info.minor << ":" << info.patch  << endl;
    std::cout << "VersionString: " << info.versionString << endl;
    
    // Currently 0 = unknown, 1 = Windows, 2 = Cygwin, 3 = Linux, 4 = Apple
    std::cout << "System: " << info.system << ": " << systemToString(info.system) << endl;
    
    // We know 32 and 64-bit systems.
    std::cout << "numBits: "  << info.numBits << endl;
    
    // Currently 0 = unknown, 1 = Microsoft Visual C++, 2 = mingw,
    // 3 = GNU g++, 4 = clang
    std::cout << "compiler: "  << info.compiler << endl;
    
    // Currently 0 = none, 1 = DllMain, 2 = Unix-style
    std::cout << "constructor: "  << info.constructor << endl;
    
    std::cout << "numCores: "  << info.numCores << endl;
    
    // Currently
    // 0 = none,
    // 1 = Windows (native),
    // 2 = OpenMP,
    // 3 = GCD,
    // 4 = Boost,
    // 5 = STL,
    // 6 = TBB,
    // 7 = STLIMPL (for_each), experimental only
    // 8 = PPLIMPL (for_each), experimental only
    std::cout << "threading: "  << info.threading << endl;
    
    // The actual number of threads configured
    std::cout << "noOfThreads: "  << info.noOfThreads << endl;
    
    // This will break if there are > 128 threads...
    // The string is of the form LLLSSS meaning 3 large TT memories
    // and 3 small ones.
    std::cout << "threadSizes: "  << info.threadSizes << endl;
    std::cout << "systemString: "  << info.systemString << endl;
    
    std::cout << "Hello again, World!\n";
    
    return 0;
}
