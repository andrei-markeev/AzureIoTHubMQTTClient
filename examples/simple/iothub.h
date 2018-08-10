#ifndef __IOTHUB_H_
#define __IOTHUB_H_

void initHubConnection();
void runHubConnection();
void sendHubEvent(const char * message);

#endif