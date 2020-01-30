#include "exodus/rpcpayload.h"

#include "exodus/createpayload.h"
#include "exodus/rpcvalues.h"
#include "exodus/rpcrequirements.h"
#include "exodus/exodus.h"
#include "exodus/sp.h"
#include "exodus/tx.h"

#include "rpc/server.h"
#include "utilstrencodings.h"

#include <univalue.h>

using std::runtime_error;
using namespace exodus;

UniValue exodus_createpayload_simplesend(const UniValue& params, bool fHelp)
{
   if (fHelp || params.size() != 2)
        throw runtime_error(
            "exodus_createpayload_simplesend propertyid \"amount\"\n"

            "\nCreate the payload for a simple send transaction.\n"

            "\nArguments:\n"
            "1. propertyid           (number, required) the identifier of the tokens to send\n"
            "2. amount               (string, required) the amount to send\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_simplesend", "1 \"100.0\"")
            + HelpExampleRpc("exodus_createpayload_simplesend", "1, \"100.0\"")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyId);
    int64_t amount = ParseAmount(params[1], isPropertyDivisible(propertyId));

    std::vector<unsigned char> payload = CreatePayload_SimpleSend(propertyId, amount);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_sendall(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "exodus_createpayload_sendall ecosystem\n"

            "\nCreate the payload for a send all transaction.\n"

            "\nArguments:\n"
            "1. ecosystem              (number, required) the ecosystem of the tokens to send (1 for main ecosystem, 2 for test ecosystem)\n"

            "\nResult:\n"
            "\"payload\"               (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_sendall", "2")
            + HelpExampleRpc("exodus_createpayload_sendall", "2")
        );

    uint8_t ecosystem = ParseEcosystem(params[0]);

    std::vector<unsigned char> payload = CreatePayload_SendAll(ecosystem);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_dexsell(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 6)
        throw runtime_error(
            "exodus_createpayload_dexsell propertyidforsale \"amountforsale\" \"amountdesired\" paymentwindow minacceptfee action\n"

            "\nCreate a payload to place, update or cancel a sell offer on the traditional distributed EXODUS/XZC exchange.\n"

            "\nArguments:\n"

            "1. propertyidforsale    (number, required) the identifier of the tokens to list for sale (must be 1 for EXODUS or 2 for TEXODUS)\n"
            "2. amountforsale        (string, required) the amount of tokens to list for sale\n"
            "3. amountdesired        (string, required) the amount of bitcoins desired\n"
            "4. paymentwindow        (number, required) a time limit in blocks a buyer has to pay following a successful accepting order\n"
            "5. minacceptfee         (string, required) a minimum mining fee a buyer has to pay to accept the offer\n"
            "6. action               (number, required) the action to take (1 for new offers, 2 to update\", 3 to cancel)\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_dexsell", "1 \"1.5\" \"0.75\" 25 \"0.0005\" 1")
            + HelpExampleRpc("exodus_createpayload_dexsell", "1, \"1.5\", \"0.75\", 25, \"0.0005\", 1")
        );

    uint32_t propertyIdForSale = ParsePropertyId(params[0]);
    uint8_t action = ParseDExAction(params[5]);

    int64_t amountForSale = 0; // depending on action
    int64_t amountDesired = 0; // depending on action
    uint8_t paymentWindow = 0; // depending on action
    int64_t minAcceptFee = 0;  // depending on action

    if (action <= CMPTransaction::UPDATE) { // actions 3 permit zero values, skip check
        amountForSale = ParseAmount(params[1], true); // TEXODUS/EXODUS is divisible
        amountDesired = ParseAmount(params[2], true); // XZC is divisible
        paymentWindow = ParseDExPaymentWindow(params[3]);
        minAcceptFee = ParseDExFee(params[4]);
    }

    std::vector<unsigned char> payload = CreatePayload_DExSell(propertyIdForSale, amountForSale, amountDesired, paymentWindow, minAcceptFee, action);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_dexaccept(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error(
            "exodus_createpayload_dexaccept propertyid \"amount\"\n"

            "\nCreate the payload for an accept offer for the specified token and amount.\n"

            "\nArguments:\n"
            "1. propertyid           (number, required) the identifier of the token to purchase\n"
            "2. amount               (string, required) the amount to accept\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_dexaccept", "1 \"15.0\"")
            + HelpExampleRpc("exodus_createpayload_dexaccept", "1, \"15.0\"")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequirePrimaryToken(propertyId);
    int64_t amount = ParseAmount(params[1], true);

    std::vector<unsigned char> payload = CreatePayload_DExAccept(propertyId, amount);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_sto(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "exodus_createpayload_sto propertyid \"amount\" ( distributionproperty )\n"

            "\nCreates the payload for a send-to-owners transaction.\n"

            "\nArguments:\n"
            "1. propertyid             (number, required) the identifier of the tokens to distribute\n"
            "2. amount                 (string, required) the amount to distribute\n"
            "3. distributionproperty   (number, optional) the identifier of the property holders to distribute to\n"
            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_sto", "3 \"5000\"")
            + HelpExampleRpc("exodus_createpayload_sto", "3, \"5000\"")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyId);
    int64_t amount = ParseAmount(params[1], isPropertyDivisible(propertyId));
    uint32_t distributionPropertyId = (params.size() > 2) ? ParsePropertyId(params[2]) : propertyId;

    std::vector<unsigned char> payload = CreatePayload_SendToOwners(propertyId, amount, distributionPropertyId);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_issuancefixed(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 9)
        throw runtime_error(
            "exodus_createpayload_issuancefixed ecosystem type previousid \"category\" \"subcategory\" \"name\" \"url\" \"data\" \"amount\"\n"

            "\nCreates the payload for a new tokens issuance with fixed supply.\n"

            "\nArguments:\n"
            "1. ecosystem            (string, required) the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)\n"
            "2. type                 (number, required) the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)\n"
            "3. previousid           (number, required) an identifier of a predecessor token (use 0 for new tokens)\n"
            "4. category             (string, required) a category for the new tokens (can be \"\")\n"
            "5. subcategory          (string, required) a subcategory for the new tokens  (can be \"\")\n"
            "6. name                 (string, required) the name of the new tokens to create\n"
            "7. url                  (string, required) an URL for further information about the new tokens (can be \"\")\n"
            "8. data                 (string, required) a description for the new tokens (can be \"\")\n"
            "9. amount               (string, required) the number of tokens to create\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_issuancefixed", "2 1 0 \"Companies\" \"Zcoin Mining\" \"Quantum Miner\" \"\" \"\" \"1000000\"")
            + HelpExampleRpc("exodus_createpayload_issuancefixed", "2, 1, 0, \"Companies\", \"Zcoin Mining\", \"Quantum Miner\", \"\", \"\", \"1000000\"")
        );

    uint8_t ecosystem = ParseEcosystem(params[0]);
    uint16_t type = ParsePropertyType(params[1]);
    uint32_t previousId = ParsePreviousPropertyId(params[2]);
    std::string category = ParseText(params[3]);
    std::string subcategory = ParseText(params[4]);
    std::string name = ParseText(params[5]);
    std::string url = ParseText(params[6]);
    std::string data = ParseText(params[7]);
    int64_t amount = ParseAmount(params[8], type);

    RequirePropertyName(name);

    std::vector<unsigned char> payload = CreatePayload_IssuanceFixed(ecosystem, type, previousId, category, subcategory, name, url, data, amount);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_issuancecrowdsale(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 13)
        throw runtime_error(
            "exodus_createpayload_issuancecrowdsale ecosystem type previousid \"category\" \"subcategory\" \"name\" \"url\" \"data\" propertyiddesired tokensperunit deadline earlybonus issuerpercentage\n"

            "\nCreates the payload for a new tokens issuance with crowdsale.\n"

            "\nArguments:\n"
            "1. ecosystem            (string, required) the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)\n"
            "2. type                 (number, required) the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)\n"
            "3. previousid           (number, required) an identifier of a predecessor token (0 for new crowdsales)\n"
            "4. category             (string, required) a category for the new tokens (can be \"\")\n"
            "5. subcategory          (string, required) a subcategory for the new tokens  (can be \"\")\n"
            "6. name                 (string, required) the name of the new tokens to create\n"
            "7. url                  (string, required) an URL for further information about the new tokens (can be \"\")\n"
            "8. data                 (string, required) a description for the new tokens (can be \"\")\n"
            "9. propertyiddesired    (number, required) the identifier of a token eligible to participate in the crowdsale\n"
            "10. tokensperunit       (string, required) the amount of tokens granted per unit invested in the crowdsale\n"
            "11. deadline            (number, required) the deadline of the crowdsale as Unix timestamp\n"
            "12. earlybonus          (number, required) an early bird bonus for participants in percent per week\n"
            "13. issuerpercentage    (number, required) a percentage of tokens that will be granted to the issuer\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_issuancecrowdsale", "2 1 0 \"Companies\" \"Zcoin Mining\" \"Quantum Miner\" \"\" \"\" 2 \"100\" 1483228800 30 2")
            + HelpExampleRpc("exodus_createpayload_issuancecrowdsale", "2, 1, 0, \"Companies\", \"Zcoin Mining\", \"Quantum Miner\", \"\", \"\", 2, \"100\", 1483228800, 30, 2")
        );

    uint8_t ecosystem = ParseEcosystem(params[0]);
    uint16_t type = ParsePropertyType(params[1]);
    uint32_t previousId = ParsePreviousPropertyId(params[2]);
    std::string category = ParseText(params[3]);
    std::string subcategory = ParseText(params[4]);
    std::string name = ParseText(params[5]);
    std::string url = ParseText(params[6]);
    std::string data = ParseText(params[7]);
    uint32_t propertyIdDesired = ParsePropertyId(params[8]);
    int64_t numTokens = ParseAmount(params[9], type);
    int64_t deadline = ParseDeadline(params[10]);
    uint8_t earlyBonus = ParseEarlyBirdBonus(params[11]);
    uint8_t issuerPercentage = ParseIssuerBonus(params[12]);

    RequirePropertyName(name);
    RequireExistingProperty(propertyIdDesired);
    RequireSameEcosystem(ecosystem, propertyIdDesired);

    std::vector<unsigned char> payload = CreatePayload_IssuanceVariable(ecosystem, type, previousId, category, subcategory, name, url, data, propertyIdDesired, numTokens, deadline, earlyBonus, issuerPercentage);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_issuancemanaged(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 8)
        throw runtime_error(
            "exodus_createpayload_issuancemanaged ecosystem type previousid \"category\" \"subcategory\" \"name\" \"url\" \"data\"\n"

            "\nCreates the payload for a new tokens issuance with manageable supply.\n"

            "\nArguments:\n"
            "1. ecosystem            (string, required) the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)\n"
            "2. type                 (number, required) the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)\n"
            "3. previousid           (number, required) an identifier of a predecessor token (use 0 for new tokens)\n"
            "4. category             (string, required) a category for the new tokens (can be \"\")\n"
            "5. subcategory          (string, required) a subcategory for the new tokens  (can be \"\")\n"
            "6. name                 (string, required) the name of the new tokens to create\n"
            "7. url                  (string, required) an URL for further information about the new tokens (can be \"\")\n"
            "8. data                 (string, required) a description for the new tokens (can be \"\")\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_issuancemanaged", "2 1 0 \"Companies\" \"Zcoin Mining\" \"Quantum Miner\" \"\" \"\"")
            + HelpExampleRpc("exodus_createpayload_issuancemanaged", "2, 1, 0, \"Companies\", \"Zcoin Mining\", \"Quantum Miner\", \"\", \"\"")
        );

    uint8_t ecosystem = ParseEcosystem(params[0]);
    uint16_t type = ParsePropertyType(params[1]);
    uint32_t previousId = ParsePreviousPropertyId(params[2]);
    std::string category = ParseText(params[3]);
    std::string subcategory = ParseText(params[4]);
    std::string name = ParseText(params[5]);
    std::string url = ParseText(params[6]);
    std::string data = ParseText(params[7]);

    RequirePropertyName(name);

    std::vector<unsigned char> payload = CreatePayload_IssuanceManaged(ecosystem, type, previousId, category, subcategory, name, url, data);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_closecrowdsale(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "exodus_createpayload_closecrowdsale propertyid\n"

            "\nCreates the payload to manually close a crowdsale.\n"

            "\nArguments:\n"
            "1. propertyid             (number, required) the identifier of the crowdsale to close\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_closecrowdsale", "70")
            + HelpExampleRpc("exodus_createpayload_closecrowdsale", "70")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);

    // checks bypassed because someone may wish to prepare the payload to close a crowdsale creation not yet broadcast

    std::vector<unsigned char> payload = CreatePayload_CloseCrowdsale(propertyId);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_grant(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "exodus_createpayload_grant propertyid \"amount\" ( \"memo\" )\n"

            "\nCreates the payload to issue or grant new units of managed tokens.\n"

            "\nArguments:\n"
            "1. propertyid           (number, required) the identifier of the tokens to grant\n"
            "2. amount               (string, required) the amount of tokens to create\n"
            "3. memo                 (string, optional) a text note attached to this transaction (none by default)\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_grant", "51 \"7000\"")
            + HelpExampleRpc("exodus_createpayload_grant", "51, \"7000\"")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    int64_t amount = ParseAmount(params[1], isPropertyDivisible(propertyId));
    std::string memo = (params.size() > 2) ? ParseText(params[2]): "";

    std::vector<unsigned char> payload = CreatePayload_Grant(propertyId, amount, memo);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_revoke(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "exodus_createpayload_revoke propertyid \"amount\" ( \"memo\" )\n"

            "\nCreates the payload to revoke units of managed tokens.\n"

            "\nArguments:\n"
            "1. propertyid           (number, required) the identifier of the tokens to revoke\n"
            "2. amount               (string, required) the amount of tokens to revoke\n"
            "3. memo                 (string, optional) a text note attached to this transaction (none by default)\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_revoke", "51 \"100\"")
            + HelpExampleRpc("exodus_createpayload_revoke", "51, \"100\"")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    int64_t amount = ParseAmount(params[1], isPropertyDivisible(propertyId));
    std::string memo = (params.size() > 2) ? ParseText(params[2]): "";

    std::vector<unsigned char> payload = CreatePayload_Revoke(propertyId, amount, memo);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_changeissuer(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "exodus_createpayload_changeissuer propertyid\n"

            "\nCreats the payload to change the issuer on record of the given tokens.\n"

            "\nArguments:\n"
            "1. propertyid           (number, required) the identifier of the tokens\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_changeissuer", "3")
            + HelpExampleRpc("exodus_createpayload_changeissuer", "3")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyId);

    std::vector<unsigned char> payload = CreatePayload_ChangeIssuer(propertyId);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_trade(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 4)
        throw runtime_error(
            "exodus_createpayload_trade propertyidforsale \"amountforsale\" propertiddesired \"amountdesired\"\n"

            "\nCreates the payload to place a trade offer on the distributed token exchange.\n"

            "\nArguments:\n"
            "1. propertyidforsale    (number, required) the identifier of the tokens to list for sale\n"
            "2. amountforsale        (string, required) the amount of tokens to list for sale\n"
            "3. propertiddesired     (number, required) the identifier of the tokens desired in exchange\n"
            "4. amountdesired        (string, required) the amount of tokens desired in exchange\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_trade", "31 \"250.0\" 1 \"10.0\"")
            + HelpExampleRpc("exodus_createpayload_trade", "31, \"250.0\", 1, \"10.0\"")
        );

    uint32_t propertyIdForSale = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyIdForSale);
    int64_t amountForSale = ParseAmount(params[1], isPropertyDivisible(propertyIdForSale));
    uint32_t propertyIdDesired = ParsePropertyId(params[2]);
    RequireExistingProperty(propertyIdDesired);
    int64_t amountDesired = ParseAmount(params[3], isPropertyDivisible(propertyIdDesired));
    RequireSameEcosystem(propertyIdForSale, propertyIdDesired);
    RequireDifferentIds(propertyIdForSale, propertyIdDesired);
    RequireDifferentIds(propertyIdForSale, propertyIdDesired);

    std::vector<unsigned char> payload = CreatePayload_MetaDExTrade(propertyIdForSale, amountForSale, propertyIdDesired, amountDesired);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_canceltradesbyprice(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 4)
        throw runtime_error(
            "exodus_createpayload_canceltradesbyprice propertyidforsale \"amountforsale\" propertiddesired \"amountdesired\"\n"

            "\nCreates the payload to cancel offers on the distributed token exchange with the specified price.\n"

            "\nArguments:\n"
            "1. propertyidforsale    (number, required) the identifier of the tokens listed for sale\n"
            "2. amountforsale        (string, required) the amount of tokens to listed for sale\n"
            "3. propertiddesired     (number, required) the identifier of the tokens desired in exchange\n"
            "4. amountdesired        (string, required) the amount of tokens desired in exchange\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_canceltradesbyprice", "31 \"100.0\" 1 \"5.0\"")
            + HelpExampleRpc("exodus_createpayload_canceltradesbyprice", "31, \"100.0\", 1, \"5.0\"")
        );

    uint32_t propertyIdForSale = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyIdForSale);
    int64_t amountForSale = ParseAmount(params[1], isPropertyDivisible(propertyIdForSale));
    uint32_t propertyIdDesired = ParsePropertyId(params[2]);
    RequireExistingProperty(propertyIdDesired);
    int64_t amountDesired = ParseAmount(params[3], isPropertyDivisible(propertyIdDesired));
    RequireSameEcosystem(propertyIdForSale, propertyIdDesired);
    RequireDifferentIds(propertyIdForSale, propertyIdDesired);

    std::vector<unsigned char> payload = CreatePayload_MetaDExCancelPrice(propertyIdForSale, amountForSale, propertyIdDesired, amountDesired);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_canceltradesbypair(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error(
            "exodus_createpayload_canceltradesbypair propertyidforsale propertiddesired\n"

            "\nCreates the payload to cancel all offers on the distributed token exchange with the given currency pair.\n"

            "\nArguments:\n"
            "1. propertyidforsale    (number, required) the identifier of the tokens listed for sale\n"
            "2. propertiddesired     (number, required) the identifier of the tokens desired in exchange\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_canceltradesbypair", "1 31")
            + HelpExampleRpc("exodus_createpayload_canceltradesbypair", "1, 31")
        );

    uint32_t propertyIdForSale = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyIdForSale);
    uint32_t propertyIdDesired = ParsePropertyId(params[1]);
    RequireExistingProperty(propertyIdDesired);
    RequireSameEcosystem(propertyIdForSale, propertyIdDesired);
    RequireDifferentIds(propertyIdForSale, propertyIdDesired);

    std::vector<unsigned char> payload = CreatePayload_MetaDExCancelPair(propertyIdForSale, propertyIdDesired);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_cancelalltrades(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "exodus_createpayload_cancelalltrades ecosystem\n"

            "\nCreates the payload to cancel all offers on the distributed token exchange.\n"

            "\nArguments:\n"
            "1. ecosystem            (number, required) the ecosystem of the offers to cancel (1 for main ecosystem, 2 for test ecosystem)\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_cancelalltrades", "1")
            + HelpExampleRpc("exodus_createpayload_cancelalltrades", "1")
        );

    uint8_t ecosystem = ParseEcosystem(params[0]);

    std::vector<unsigned char> payload = CreatePayload_MetaDExCancelEcosystem(ecosystem);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_enablefreezing(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "exodus_createpayload_enablefreezing propertyid\n"

            "\nCreates the payload to enable address freezing for a centrally managed property.\n"

            "\nArguments:\n"
            "1. propertyid           (number, required) the identifier of the tokens\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_enablefreezing", "3")
            + HelpExampleRpc("exodus_createpayload_enablefreezing", "3")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);

    std::vector<unsigned char> payload = CreatePayload_EnableFreezing(propertyId);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_disablefreezing(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "exodus_createpayload_disablefreezing propertyid\n"

            "\nCreates the payload to disable address freezing for a centrally managed property.\n"
            "\nIMPORTANT NOTE:  Disabling freezing for a property will UNFREEZE all frozen addresses for that property!"

            "\nArguments:\n"
            "1. propertyid           (number, required) the identifier of the tokens\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_disablefreezing", "3")
            + HelpExampleRpc("exodus_createpayload_disablefreezing", "3")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);

    std::vector<unsigned char> payload = CreatePayload_DisableFreezing(propertyId);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_freeze(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 3)
        throw runtime_error(
            "exodus_createpayload_freeze \"toaddress\" propertyid amount \n"

            "\nCreates the payload to freeze an address for a centrally managed token.\n"

            "\nArguments:\n"
            "1. toaddress            (string, required) the address to freeze tokens for\n"
            "2. propertyid           (number, required) the property to freeze tokens for (must be managed type and have freezing option enabled)\n"
            "3. amount               (number, required) the amount of tokens to freeze (note: this is unused - once frozen an address cannot send any transactions)\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_freeze", "\"3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs\" 1 0")
            + HelpExampleRpc("exodus_createpayload_freeze", "\"3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs\", 1, 0")
        );

    std::string refAddress = ParseAddress(params[0]);
    uint32_t propertyId = ParsePropertyId(params[1]);
    int64_t amount = ParseAmount(params[2], isPropertyDivisible(propertyId));

    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);

    std::vector<unsigned char> payload = CreatePayload_FreezeTokens(propertyId, amount, refAddress);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_unfreeze(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 3)
        throw runtime_error(
            "exodus_createpayload_unfreeze \"toaddress\" propertyid amount \n"

            "\nCreates the payload to unfreeze an address for a centrally managed token.\n"

            "\nArguments:\n"
            "1. toaddress            (string, required) the address to unfreeze tokens for\n"
            "2. propertyid           (number, required) the property to unfreeze tokens for (must be managed type and have freezing option enabled)\n"
            "3. amount               (number, required) the amount of tokens to unfreeze (note: this is unused)\n"

            "\nResult:\n"
            "\"payload\"             (string) the hex-encoded payload\n"

            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_unfreeze", "\"3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs\" 1 0")
            + HelpExampleRpc("exodus_createpayload_unfreeze", "\"3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs\", 1, 0")
        );

    std::string refAddress = ParseAddress(params[0]);
    uint32_t propertyId = ParsePropertyId(params[1]);
    int64_t amount = ParseAmount(params[2], isPropertyDivisible(propertyId));

    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);

    std::vector<unsigned char> payload = CreatePayload_UnfreezeTokens(propertyId, amount, refAddress);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_createdenomination(const UniValue& params, bool fHelp)
{

    if (fHelp || params.size() != 2)
        throw std::runtime_error(
            "exodus_createpayload_createdenomination propertyid \"value\"\n"
            "\nCreate a payload for create a denomination for the given property.\n"
            "\nArguments:\n"
            "1. propertyid           (number, required) the property to create a new denomination\n"
            "2. value                (string, required) the value of denomination to create\n"
            "\nResult:\n"
            "\"hash\"                  (string) the hex-encoded payload\n"
            "\nExamples:\n"
            + HelpExampleCli("exodus_createpayload_createdenomination", "1 \"100.0\"")
            + HelpExampleRpc("exodus_createpayload_createdenomination", "1, \"100.0\"")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    int64_t value = ParseAmount(params[1], isPropertyDivisible(propertyId));

    RequireExistingProperty(propertyId);
    RequireSigma(propertyId);

    // validate
    {
        LOCK(cs_main);

        CMPSPInfo::Entry info;
        assert(_my_sps->getSP(propertyId, info));

        if (info.denominations.size() >= MAX_DENOMINATIONS) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "No more room for new denomination");
        }

        if (std::find(info.denominations.begin(), info.denominations.end(), value) != info.denominations.end()) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Denomination with value " + FormatMP(propertyId, value) + " already exists");
        }
    }

    // create a payload
    auto payload = CreatePayload_CreateDenomination(propertyId, value);

    return HexStr(payload.begin(), payload.end());
}

UniValue exodus_createpayload_mintbypublickeys(const UniValue& params, bool fHelp)
{

    if (fHelp || params.size() < 2 || params.size() > 3)
        throw std::runtime_error(
            "exodus_sendmint \"fromaddress\" propertyid {\"denomination\":amount,...} ( denomminconf )\n"
            "\nCreate mints.\n"
            "\nArguments:\n"
            "1. propertyid                          (number, required) the property to create mints\n"
            "2. mints                               (string, required) a JSON array of pairs of public key and denomination\n"
            "     [\n"
            "       {\n"
            "         \"id\":\"hex\"                    (string, required) public key of coin to create mint\n"
            "         \"denomination\":n              (number, required) denomination to create coin\n"
            "       }\n"
            "       ,...\n"
            "     ]\n"
            "3. denomminconf                        (number, optional, default=6) Allow only denominations with at least this many confirmations\n"
            "\nResult:\n"
            "\"hash\"                          (string) the hex-encoded payload\n"
            "\nExamples:\n"
            + HelpExampleCli("exodus_sendmint", "1 \"[{\"id\":\"52cd0023a3a40b91201d199f9f1623125371b20256957325bf210b5492a8eb9c0100\", \"denomination\":0}]\"")
            + HelpExampleRpc("exodus_sendmint", "1, \"[{\"id\":\"52cd0023a3a40b91201d199f9f1623125371b20256957325bf210b5492a8eb9c0100\", \"denomination\":0}]\"")
        );

    uint32_t propertyId = ParsePropertyId(params[0]);
    UniValue mintObjs(UniValue::VARR);
    mintObjs = params[1].get_array();

    RequireExistingProperty(propertyId);
    RequireSigma(propertyId);

    std::vector<pair<uint8_t, SigmaPublicKey>> mints;
    for (size_t idx = 0; idx < mintObjs.size(); idx++) {
        const auto &mint = mintObjs[idx].get_obj();

        RPCTypeCheckObj(mint,
            {
                {"id", UniValueType(UniValue::VSTR)},
                {"denomination", UniValueType(UniValue::VNUM)}
            });


        auto id = ParseHex(find_value(mint, "id").get_str());
        const auto denomId = find_value(mint, "denomination").get_int();

        CDataStream pubkeyDeserialized(id, SER_NETWORK, CLIENT_VERSION);
        SigmaPublicKey key;
        try {
            pubkeyDeserialized >> key;
        } catch (const std::ios_base::failure &)
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Public key is invalid.");
        }

        if (!key.IsValid()) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Public key is invalid.");
        }

        if (denomId < 0 || denomId > UINT8_MAX) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Denomination id is invalid.");
        }

        mints.push_back(std::make_pair(static_cast<SigmaDenomination>(denomId), key));
    }

    // validate denominations.
    {
        LOCK(cs_main);

        CMPSPInfo::Entry info;
        assert(_my_sps->getSP(propertyId, info));

        for (auto &mint : mints) {
            if (mint.first >= info.denominations.size()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Denomination is not exist");
            }
        }
    }

    auto payload = CreatePayload_SimpleMint(propertyId, mints);

    return HexStr(payload.begin(), payload.end());
}

static const CRPCCommand commands[] =
{ //  category                         name                                      actor (function)                         okSafeMode
  //  -------------------------------- ----------------------------------------- ---------------------------------------- ----------
    { "exodus (payload creation)", "exodus_createpayload_simplesend",          &exodus_createpayload_simplesend,          true },
    { "exodus (payload creation)", "exodus_createpayload_sendall",             &exodus_createpayload_sendall,             true },
    { "exodus (payload creation)", "exodus_createpayload_dexsell",             &exodus_createpayload_dexsell,             true },
    { "exodus (payload creation)", "exodus_createpayload_dexaccept",           &exodus_createpayload_dexaccept,           true },
    { "exodus (payload creation)", "exodus_createpayload_sto",                 &exodus_createpayload_sto,                 true },
    { "exodus (payload creation)", "exodus_createpayload_grant",               &exodus_createpayload_grant,               true },
    { "exodus (payload creation)", "exodus_createpayload_revoke",              &exodus_createpayload_revoke,              true },
    { "exodus (payload creation)", "exodus_createpayload_changeissuer",        &exodus_createpayload_changeissuer,        true },
    { "exodus (payload creation)", "exodus_createpayload_trade",               &exodus_createpayload_trade,               true },
    { "exodus (payload creation)", "exodus_createpayload_issuancefixed",       &exodus_createpayload_issuancefixed,       true },
    { "exodus (payload creation)", "exodus_createpayload_issuancecrowdsale",   &exodus_createpayload_issuancecrowdsale,   true },
    { "exodus (payload creation)", "exodus_createpayload_issuancemanaged",     &exodus_createpayload_issuancemanaged,     true },
    { "exodus (payload creation)", "exodus_createpayload_closecrowdsale",      &exodus_createpayload_closecrowdsale,      true },
    { "exodus (payload creation)", "exodus_createpayload_canceltradesbyprice", &exodus_createpayload_canceltradesbyprice, true },
    { "exodus (payload creation)", "exodus_createpayload_canceltradesbypair",  &exodus_createpayload_canceltradesbypair,  true },
    { "exodus (payload creation)", "exodus_createpayload_cancelalltrades",     &exodus_createpayload_cancelalltrades,     true },
    { "exodus (payload creation)", "exodus_createpayload_enablefreezing",      &exodus_createpayload_enablefreezing,      true },
    { "exodus (payload creation)", "exodus_createpayload_disablefreezing",     &exodus_createpayload_disablefreezing,     true },
    { "exodus (payload creation)", "exodus_createpayload_freeze",              &exodus_createpayload_freeze,              true },
    { "exodus (payload creation)", "exodus_createpayload_unfreeze",            &exodus_createpayload_unfreeze,            true },
    { "exodus (payload creation)", "exodus_createpayload_createdenomination",  &exodus_createpayload_createdenomination,  true },
    { "exodus (payload creation)", "exodus_createpayload_mintbypublickeys",    &exodus_createpayload_mintbypublickeys,    true },
};

void RegisterExodusPayloadCreationRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
