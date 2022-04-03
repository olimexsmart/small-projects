/*
	HTTPparser for Arduino
*/


#include "HTTPparser.h"

HTTPparser::HTTPparser() {
    pathAllocation = PATH_ALLOCATION;
    messageAllocation = MSG_ALLOCATION;
    Reset();
}

void HTTPparser::Reset() {
    Method = NR;
    index = 0;
    status = METHOD;
}

void HTTPparser::AllSheWrote() {
    if (status == ERROR)
        return;

    // Null teminating Message
    Message[index] = '\0';

    status = DONE;
}

bool HTTPparser::IsValid() {
    return status == DONE;
}

const char * HTTPparser::MethodString() {
    return methodStrings[Method];
}


void HTTPparser::ParseChar(char c) {
    switch (status) {
        case METHOD:
            if (c == ' ') { // Need to switch status
                if (strcmp(method, "GET") == 0)
                    Method = GET;
                if (strcmp(method, "POST") == 0)
                    Method = POST;
                // If it is neither of them we have a problem
                if (Method == NR) {
                    status = ERROR;
                    return;
                }
                status = PATH;
                index = 0;
                return;
            }
            // Saving the new char
            method[index] = c;
            index++;
            if (index == 5)
                status = ERROR;
            else
                method[index] = '\0';
            break;

        case PATH:
            if (c == ' ') {
                Path[index] = '\0';
                status = HEADERS;
                index = 0;
                return;
            }
            // Saving the new char
            Path[index] = c;
            index++;
            if (index == pathAllocation)
                status = ERROR;
            break;

        case HEADERS:
            // Ignoring everything but newlines
            if (c == '\n')
                status = BLANK;
            break;

        case BLANK:
            // If \r stays in BLANK
            if (c != '\r')
                status = HEADERS;
            if (c == '\n')
                status = MESSAGE;
            break;

        case MESSAGE:
            Message[index] = c;
            index++;
            if (index == messageAllocation) {
                status = FULL;
                index--; // Preparation for null termination
            }
            break;

        default:
            // To get rid of the warning
            break;
    }
}
