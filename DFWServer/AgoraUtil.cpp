#include <string>
#include <ctime>

#include "Agora/RtcTokenBuilder.h"

using namespace std;

string generateToken(string& channel) {
	string appId = "";
	string appCertificate = "";
	
	uint32_t uid = 0;
    uint32_t expirationTimeInSeconds = 3600;
    uint32_t currentTimeStamp = time(NULL);
    uint32_t privilegeExpiredTs = currentTimeStamp + expirationTimeInSeconds;
    std::string result;

    result = RtcTokenBuilder::buildTokenWithUid(
        appID, appCertificate, channelName, uid, UserRole::Role_Publisher,
        privilegeExpiredTs);
    std::cout << "Token With Int Uid:" << result << std::endl;
}