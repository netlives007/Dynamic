// Copyright (c) 2018 Duality Blockchain Solutions Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bdap/utils.h"

#include "chainparams.h"
#include "coins.h"
#include "policy/policy.h"
#include "serialize.h"
#include "uint256.h"
#include "validation.h"
#include "wallet/wallet.h"

#include <univalue.h>

namespace BDAP {
    std::string GetObjectTypeString(unsigned int nObjectType)
    {
        switch ((BDAP::ObjectType)nObjectType) {

            case BDAP::ObjectType::BDAP_DEFAULT_TYPE:
                return "Default";
             case BDAP::ObjectType::BDAP_USER:
                return "User Entry";
            case BDAP::ObjectType::BDAP_GROUP:
                return "Group Entry";
            case BDAP::ObjectType::BDAP_DEVICE:
                return "Device Entry";
            case BDAP::ObjectType::BDAP_DOMAIN:
                return "Domain Entry";
            case BDAP::ObjectType::BDAP_ORGANIZATIONAL_UNIT:
                return "OU Entry";
            case BDAP::ObjectType::BDAP_CERTIFICATE:
                return "Certificate Entry";
            case BDAP::ObjectType::BDAP_AUDIT:
                return "Audit Entry";
            case BDAP::ObjectType::BDAP_CHANNEL:
                return "Channel Entry";
            case BDAP::ObjectType::BDAP_CHECKPOINT:
                return "Channel Checkpoint Entry";
            case BDAP::ObjectType::BDAP_LINK_REQUEST:
                return "Link Request Entry";
            case BDAP::ObjectType::BDAP_LINK_ACCEPT:
                return "Link Accept Entry";
            case BDAP::ObjectType::BDAP_IDENTITY:
                return "Identity Entry";
            case BDAP::ObjectType::BDAP_IDENTITY_VERIFICATION:
                return "Identity Verification Entry";
            default:
                return "Unknown";
        }
    }
    
    unsigned int GetObjectTypeInt(BDAP::ObjectType ObjectType)
    {
        return (unsigned int)ObjectType;
    }

    BDAP::ObjectType GetObjectTypeEnum(unsigned int nObjectType)
    {
        return (BDAP::ObjectType)nObjectType;
    }
}

std::string BDAPFromOp(const int op) 
{
    switch (op) {
        case OP_BDAP_NEW:
            return "bdap_new";
        case OP_BDAP_DELETE:
            return "bdap_delete";
        case OP_BDAP_REVOKE:
            return "bdap_revoke";
        case OP_BDAP_MODIFY:
            return "bdap_update";
        case OP_BDAP_MODIFY_RDN:
            return "bdap_move";
        case OP_BDAP_LINK_REQUEST:
            return "bdap_link_request";
        case OP_BDAP_LINK_ACCEPT:
            return "bdap_link_accept";
        case OP_BDAP_AUDIT:
            return "bdap_audit";
        case OP_BDAP_CERTIFICATE:
            return "bdap_certificate";
        case OP_BDAP_IDENTITY:
            return "bdap_identity";
        case OP_BDAP_ID_VERIFICATION:
            return "bdap_identity_verification";
        case OP_BDAP_CHANNEL:
            return "bdap_new_channel";
        case OP_BDAP_CHANNEL_CHECKPOINT:
            return "bdap_channel_checkpoint";
        default:
            return "<unknown bdap op>";
    }
}

bool IsBDAPDataOutput(const CTxOut& out) {
    txnouttype whichType;
    if (!IsStandard(out.scriptPubKey, whichType))
        return false;
    if (whichType == TX_NULL_DATA)
        return true;
   return false;
}

bool GetBDAPTransaction(int nHeight, const uint256& hash, CTransactionRef &txOut, const Consensus::Params& consensusParams)
{
    if(nHeight < 0 || nHeight > chainActive.Height())
        return false;

    CBlockIndex *pindexSlow = NULL;

    LOCK(cs_main);
    
    pindexSlow = chainActive[nHeight];
    
    if (pindexSlow) {
        CBlock block;
        if (ReadBlockFromDisk(block, pindexSlow, consensusParams)) {
                for (const auto& tx : block.vtx) {
                if (tx->GetHash() == hash) {
                    txOut = tx;
                    return true;
                }
            }
        }
    }
    return false;
}

std::string stringFromVch(const CharString& vch) {
    std::string res;
    std::vector<unsigned char>::const_iterator vi = vch.begin();
    while (vi != vch.end()) {
        res += (char) (*vi);
        vi++;
    }
    return res;
}

std::vector<unsigned char> vchFromValue(const UniValue& value) {
    std::string strName = value.get_str();
    unsigned char *strbeg = (unsigned char*) strName.c_str();
    return std::vector<unsigned char>(strbeg, strbeg + strName.size());
}

std::vector<unsigned char> vchFromString(const std::string& str) 
{
    return std::vector<unsigned char>(str.begin(), str.end());
}

int GetBDAPDataOutput(const CTransactionRef& tx) {
   for(unsigned int i = 0; i < tx->vout.size();i++) {
       if(IsBDAPDataOutput(tx->vout[i]))
           return i;
    }
   return -1;
}

bool GetBDAPData(const CScript& scriptPubKey, std::vector<unsigned char>& vchData, std::vector<unsigned char>& vchHash)
{
    CScript::const_iterator pc = scriptPubKey.begin();
    opcodetype opcode;
    if (!scriptPubKey.GetOp(pc, opcode))
        return false;
    if(opcode != OP_RETURN)
        return false;
    if (!scriptPubKey.GetOp(pc, opcode, vchData))
        return false;

    uint256 hash;
    hash = Hash(vchData.begin(), vchData.end());
    vchHash = vchFromValue(hash.GetHex());

    return true;
}

bool GetBDAPData(const CTransactionRef& tx, std::vector<unsigned char>& vchData, std::vector<unsigned char>& vchHash, int& nOut)
{
    nOut = GetBDAPDataOutput(tx);
    if(nOut == -1)
       return false;

    const CScript &scriptPubKey = tx->vout[nOut].scriptPubKey;
    return GetBDAPData(scriptPubKey, vchData, vchHash);
}

bool GetBDAPData(const CTxOut& out, std::vector<unsigned char>& vchData, std::vector<unsigned char>& vchHash)
{
    return GetBDAPData(out.scriptPubKey, vchData, vchHash);
}

void CreateRecipient(const CScript& scriptPubKey, CRecipient& recipient)
{
    CRecipient recp = {scriptPubKey, recipient.nAmount, false};
    recipient = recp;
    CTxOut txout(recipient.nAmount, scriptPubKey);
    size_t nSize = GetSerializeSize(txout, SER_DISK, 0) + 148u;
    recipient.nAmount = 3 * minRelayTxFee.GetFee(nSize);
}

void CreateFeeRecipient(CScript& scriptPubKey, const std::vector<unsigned char>& data, CRecipient& recipient)
{
    // add hash to data output (must match hash in inputs check with the tx scriptpubkey hash)
    uint256 hash = Hash(data.begin(), data.end());
    std::vector<unsigned char> vchHashRand = vchFromValue(hash.GetHex());
    scriptPubKey << vchHashRand;
    CRecipient recp = {scriptPubKey, 0, false};
    recipient = recp;
}

CAmount GetDataFee(const CScript& scriptPubKey)
{
    CAmount nFee = 0;
    CRecipient recp = {scriptPubKey, 0, false};
    CTxOut txout(0, scriptPubKey);
    size_t nSize = GetSerializeSize(txout, SER_DISK,0)+148u;
    nFee = CWallet::GetMinimumFee(nSize, nTxConfirmTarget, mempool);
    recp.nAmount = nFee;
    return recp.nAmount;
}

void ToLowerCase(CharString& vchValue) {
    std::string strValue;
    CharString::const_iterator vi = vchValue.begin();
    while (vi != vchValue.end()) 
    {
        strValue += std::tolower(*vi);
        vi++;
    }
    CharString vchNewValue(strValue.begin(), strValue.end());
    std::swap(vchValue, vchNewValue);
}

void ToLowerCase(std::string& strValue) {
    for(unsigned short loop=0;loop < strValue.size();loop++)
    {
        strValue[loop]=std::tolower(strValue[loop]);
    }
}

CAmount GetBDAPFee(const CScript& scriptPubKey)
{
    CAmount nFee = 0;
    CRecipient recp = {scriptPubKey, 0, false};
    CTxOut txout(0, scriptPubKey);
    size_t nSize = GetSerializeSize(txout, SER_DISK,0)+148u;
    nFee = CWallet::GetMinimumFee(nSize, nTxConfirmTarget, mempool);
    recp.nAmount = nFee;
    return recp.nAmount;
}

bool DecodeBDAPTx(const CTransactionRef& tx, int& op, std::vector<std::vector<unsigned char> >& vvch) 
{
    bool found = false;

    for (unsigned int i = 0; i < tx->vout.size(); i++) {
        const CTxOut& out = tx->vout[i];
        vchCharString vvchRead;
        if (DecodeBDAPScript(out.scriptPubKey, op, vvchRead)) {
            found = true;
            vvch = vvchRead;
            break;
        }
    }
    if (!found)
        vvch.clear();

    return found;
}

bool FindBDAPInTx(const CCoinsViewCache &inputs, const CTransaction& tx, std::vector<std::vector<unsigned char> >& vvch)
{
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        const Coin& prevCoins = inputs.AccessCoin(tx.vin[i].prevout);
        if (prevCoins.IsSpent()) {
            continue;
        }
        // check unspent input for consensus before adding to a block
        int op;
        if (DecodeBDAPScript(prevCoins.out.scriptPubKey, op, vvch)) {
            return true;
        }
    }
    return false;
}

int GetBDAPOpType(const CScript& script)
{
    CScript::const_iterator it = script.begin();
    opcodetype op1 = OP_INVALIDOPCODE;
    opcodetype op2 = OP_INVALIDOPCODE;
    while (it != script.end()) {
        std::vector<unsigned char> vch;
        if (op1 == OP_INVALIDOPCODE)
        {
            if (script.GetOp2(it, op1, &vch)) 
            {
                if (op1 - OP_1NEGATE - 1 == OP_BDAP)
                {
                    continue;
                }
                else
                {
                    return 0;
                }
            }
        }
        else
        {
            if (script.GetOp2(it, op2, &vch)) 
            {
                if (op2 - OP_1NEGATE - 1  > OP_BDAP && op2 - OP_1NEGATE - 1 <= OP_BDAP_CHANNEL_CHECKPOINT)
                {
                    return (int)op2 - OP_1NEGATE - 1;
                }
                else
                {
                    return -1;
                }
            }
        }
    }
    return (int)op2;
}

int GetBDAPOpType(const CTxOut& out)
{
    return GetBDAPOpType(out.scriptPubKey);
}

std::string GetBDAPOpTypeString(const CScript& script)
{
    return BDAPFromOp(GetBDAPOpType(script));
}

bool GetBDAPOpScript(const CTransactionRef& tx, CScript& scriptBDAPOp, vchCharString& vvchOpParameters, int& op)
{
    for (unsigned int i = 0; i < tx->vout.size(); i++) 
    {
        const CTxOut& out = tx->vout[i];
        if (DecodeBDAPScript(out.scriptPubKey, op, vvchOpParameters)) 
        {
            scriptBDAPOp = out.scriptPubKey;
            return true;
        }
    }
    return false;
}

bool GetBDAPOpScript(const CTransactionRef& tx, CScript& scriptBDAPOp)
{
    int op;
    vchCharString vvchOpParameters;
    return GetBDAPOpScript(tx, scriptBDAPOp, vvchOpParameters, op);
}

bool GetBDAPDataScript(const CTransaction& tx, CScript& scriptBDAPData)
{
    for (unsigned int i = 0; i < tx.vout.size(); i++) 
    {
        const CTxOut& out = tx.vout[i];
        if (out.scriptPubKey.IsUnspendable()) 
        {
            scriptBDAPData = out.scriptPubKey;
            return true;
        }
    }
    return false;
}

bool IsBDAPOperationOutput(const CTxOut& out)
{
    if (GetBDAPOpType(out.scriptPubKey) > 0)
        return true;
    return false;
}

int GetBDAPOpCodeFromOutput(const CTxOut& out)
{
    if (!IsBDAPOperationOutput(out)) {
        return 0;
    }

    return GetBDAPOpType(out.scriptPubKey);
}

std::string GetBDAPOpStringFromOutput(const CTxOut& out)
{
    return GetBDAPOpTypeString(out.scriptPubKey);
}

int GetBDAPOperationOutIndex(const CTransactionRef& tx) 
{
    for(unsigned int i = 0; i < tx->vout.size();i++) {
        if(IsBDAPOperationOutput(tx->vout[i]))
            return i;
    }
    return -1;
}

int GetBDAPOperationOutIndex(int nHeight, const uint256& txHash) 
{
    CTransactionRef tx;
    const Consensus::Params& consensusParams = Params().GetConsensus();
    if (!GetBDAPTransaction(nHeight, txHash, tx, consensusParams))
    {
        return -1;
    }
    return GetBDAPOperationOutIndex(tx);
}

CDynamicAddress GetScriptAddress(const CScript& pubScript)
{
    CTxDestination txDestination;
    ExtractDestination(pubScript, txDestination);
    CDynamicAddress entryAddress(txDestination);
    return entryAddress;
}