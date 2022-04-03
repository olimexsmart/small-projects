/*
	HTTPparser for Arduino
*/
#ifndef HTTP_parser
#define HTTP_parser

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#define PATH_ALLOCATION 20
#define MSG_ALLOCATION 45

class HTTPparser {

    public:
        HTTPparser();
        void ParseChar(char c);
        void Reset();
        bool IsValid();
        void AllSheWrote();
        const char * MethodString();

        char Path[PATH_ALLOCATION];
        char Message[MSG_ALLOCATION];

        typedef enum {
            GET,
            POST,
            NR
        } MethodType;
        MethodType Method;

    private:
        //HTTPparser();
        char method[6];
        byte index;
        byte pathAllocation;
        byte messageAllocation;

        typedef enum {
            ERROR,
            DONE,
            METHOD,
            PATH,
            HEADERS,
            BLANK,
            MESSAGE,
            FULL
        } Status;

        Status status;

        const char * methodStrings[3] = {
            "GET",
            "POST",
            "NR"
        };

};

#endif
