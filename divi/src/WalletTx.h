#ifndef WALLET_TX_H
#define WALLET_TX_H

#include <merkletx.h>
#include <utilstrencodings.h>
#include <tinyformat.h>
#include <amount.h>
#include <vector>
#include <utility>
#include <serialize.h>
#include <string>
#include <map>
#include <stdint.h>
class I_MerkleTxConfirmationNumberCalculator;

typedef std::map<std::string, std::string> mapValue_t;
void ReadOrderPos(int64_t& nOrderPos, mapValue_t& mapValue);
void WriteOrderPos(const int64_t& nOrderPos, mapValue_t& mapValue);

/**
 * A transaction with a bunch of additional info that only the owner cares about.
 * It includes any unrecorded transactions needed to link it back to the block chain.
 */
class CWalletTx final: public CMerkleTx
{
private:
    const I_MerkleTxConfirmationNumberCalculator& confirmationsCalculator_;
public:
    std::map<std::string, std::string> mapValue;
    std::vector<std::pair<std::string, std::string> > vOrderForm;
    unsigned int fTimeReceivedIsTxTime;
    unsigned int nTimeReceived; //! time received by this node
    unsigned int nTimeSmart;
    char createdByMe;
    std::string strFromAccount;
    int64_t nOrderPos; //! position in ordered transaction list

    // memory only
    mutable bool fDebitCached;
    mutable bool fCreditCached;
    mutable bool fImmatureCreditCached;
    mutable bool fAvailableCreditCached;
    mutable bool fWatchDebitCached;
    mutable bool fWatchCreditCached;
    mutable bool fImmatureWatchCreditCached;
    mutable bool fAvailableWatchCreditCached;
    mutable bool fChangeCached;
    mutable CAmount nDebitCached;
    mutable CAmount nCreditCached;
    mutable CAmount nImmatureCreditCached;
    mutable CAmount nAvailableCreditCached;
    mutable CAmount nWatchDebitCached;
    mutable CAmount nWatchCreditCached;
    mutable CAmount nImmatureWatchCreditCached;
    mutable CAmount nAvailableWatchCreditCached;
    mutable CAmount nChangeCached;

    CWalletTx(const CTransaction& txIn, const I_MerkleTxConfirmationNumberCalculator& confirmationsCalculator);
    void Init();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (ser_action.ForRead())
            Init();
        char fSpent = false;

        if (!ser_action.ForRead()) {
            mapValue["fromaccount"] = strFromAccount;

            WriteOrderPos(nOrderPos, mapValue);

            if (nTimeSmart)
                mapValue["timesmart"] = strprintf("%u", nTimeSmart);
        }

        READWRITE(*(CMerkleTx*)this);
        if (!ser_action.ForRead())
        {
            uint64_t unused = 0;
            WriteCompactSize(s,unused);
        }
        else
        {
            ReadCompactSize(s);
        }
        READWRITE(mapValue);
        READWRITE(vOrderForm);
        READWRITE(fTimeReceivedIsTxTime);
        READWRITE(nTimeReceived);
        READWRITE(createdByMe);
        READWRITE(fSpent);

        if (ser_action.ForRead()) {
            strFromAccount = mapValue["fromaccount"];

            ReadOrderPos(nOrderPos, mapValue);

            nTimeSmart = mapValue.count("timesmart") ? (unsigned int)atoi64(mapValue["timesmart"]) : 0;
        }

        mapValue.erase("fromaccount");
        mapValue.erase("version");
        mapValue.erase("spent");
        mapValue.erase("n");
        mapValue.erase("timesmart");
    }

    //! make sure balances are recalculated
    void RecomputeCachedQuantities();

    int64_t GetTxTime() const;
    int64_t GetComputedTxTime() const;
    /**
     * Return first confirmation block index and depth of transaction in blockchain:
     * -1  : not in blockchain, and not in memory pool (conflicted transaction)
     *  0  : in memory pool, waiting to be included in a block
     * >=1 : this many blocks deep in the main chain
     */
    int GetNumberOfBlockConfirmations() const;
    int GetBlocksToMaturity() const;
};
#endif// WALLET_TX_H