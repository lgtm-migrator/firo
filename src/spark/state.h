// Copyright (c) 2022 The Firo Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef _MAIN_SPARK_STATE_H_
#define _MAIN_SPARK_STATE_H_

#include "libspark/coin.h"
#include "chain.h"
#include "../libspark/mint_transaction.h"
#include "primitives.h"

namespace spark {

// Spark transaction info, added to the CBlock to ensure spark mint/spend transactions got their info stored into index
class CSparkTxInfo {
public:
    // all the spark transactions encountered so far
    std::set<uint256> spTransactions;

    // Vector of all mints
    std::vector<spark::Coin> mints;

    // linking tag for every spend (map from lTag to coin group id)
    std::unordered_map<GroupElement, int> spentLTags;

    // information about transactions in the block is complete
    bool fInfoIsComplete;

    CSparkTxInfo(): fInfoIsComplete(false) {}

    // finalize everything
    void Complete();
};

// check if spark activation block is passed
bool IsSparkAllowed();
bool IsSparkAllowed(int height);

// Pass Scripts form mint transaction and get spark MintTransaction object
void ParseSparkMintTransaction(const std::vector<CScript>& scripts, MintTransaction& mintTransaction);
void ParseSparkMintCoin(const CScript& script, spark::Coin& txCoin);


bool CheckSparkTransaction(
        const CTransaction &tx,
        CValidationState &state,
        uint256 hashTx,
        bool isVerifyDB,
        int nHeight,
        bool isCheckWallet,
        bool fStatefulSigmaCheck,
        CSparkTxInfo* sparkTxInfo);

bool GetOutPoint(COutPoint& outPoint, const spark::Coin& coin);
bool GetOutPoint(COutPoint& outPoint, const uint256& coinHash);
bool GetOutPointFromBlock(COutPoint& outPoint, const spark::Coin& coin, const CBlock &block);


class CSparkMempoolState {
private:
    // mints in the mempool
    std::unordered_set<spark::Coin, spark::CoinHash> mempoolMints;

    // linking tags of spends currently in the mempool mapped to tx hashes
    std::unordered_map<GroupElement, uint256, spark::CLTagHash> mempoolLTags;

public:
    // Check if there is a conflicting tx in the blockchain or mempool
    bool HasMint(const spark::Coin& coin);
    void AddMintToMempool(const spark::Coin& coin);
    void RemoveMintFromMempool(const spark::Coin& coin);

    // Check if there is a conflicting tx in the blockchain or mempool
    bool HasLTag(const GroupElement& lTag);

    // Add spend into the mempool.
    bool AddSpendToMempool(const GroupElement& lTag, uint256 txHash);

    // Remove spend from the mempool (usually as the result of adding tx to the block)
    void RemoveSpendFromMempool(const GroupElement& lTag);

    // Get conflicting tx hash by coin serial number
    uint256 GetMempoolConflictingTxHash(const GroupElement& lTag);

    std::unordered_map<GroupElement, uint256, spark::CLTagHash> const & GetMempoolLTags() const { return mempoolLTags; }

    void Reset();
};

/*
 * State of minted/spent coins as extracted from the index
 */
class CSparkState {
public:
    // First and last block where mint with given id was seen
    struct SparkCoinGroupInfo {
        SparkCoinGroupInfo() : firstBlock(nullptr), lastBlock(nullptr), nCoins(0) {}

        // first and last blocks having coins with given id minted
        CBlockIndex *firstBlock;
        CBlockIndex *lastBlock;
        // total number of minted coins with such parameters
        int nCoins;
    };

public:
    CSparkState(
            size_t maxCoinInGroup = ZC_LELANTUS_MAX_MINT_NUM,
            size_t startGroupSize = ZC_LELANTUS_SET_START_SIZE);

    // Reset to initial values
    void Reset();

    // Query if the coin linking tag was previously used
    bool IsUsedLTag(const GroupElement& lTag);
    // Query if the hash of a linking tag was previously used. If so, store preimage in coinSerial param
    bool IsUsedLTagHash(GroupElement& lTag, const uint256 &coinLTaglHash);

    // Return height of mint transaction and id of minted coin
    std::pair<int, int> GetMintedCoinHeightAndId(const spark::Coin& coin);

    // Query if there is a coin with given pubCoin value
    bool HasCoin(const spark::Coin& coin);

    // Query if there is a coin with given hash of a coin value.
    bool HasCoinHash(spark::Coin& coin, const uint256& coinHash);

    // Query coin group with given id
    bool GetCoinGroupInfo(int group_id, SparkCoinGroupInfo &result);

    int GetLatestCoinID() const;

    // Check if there is a conflicting tx in the blockchain or mempool
    bool CanAddSpendToMempool(const GroupElement& lTag);

    bool CanAddMintToMempool(const spark::Coin& coin);

    void AddMint(const spark::Coin& coin, const CMintedCoinInfo& coinInfo);
    void RemoveMint(const spark::Coin& coin);

    void AddSpend(const GroupElement& lTag, int coinGroupId);
    void RemoveSpend(const GroupElement& lTag);

    // Add spend into the mempool.
    // Check if there is a coin with such serial in either blockchain or mempool
    bool AddSpendToMempool(const std::vector<GroupElement>& lTags, uint256 txHash);

    void AddMintsToMempool(const std::vector<spark::Coin>& coins);
    void RemoveMintFromMempool(const spark::Coin& coin);

    // Get conflicting tx hash by coin linking tag
    uint256 GetMempoolConflictingTxHash(const GroupElement& lTag);

    // Remove spend from the mempool (usually as the result of adding tx to the block)
    void RemoveSpendFromMempool(const std::vector<GroupElement>& lTags);

    std::unordered_map<spark::Coin, CMintedCoinInfo, spark::CoinHash> const & GetMints() const;
    std::unordered_map<GroupElement, int, spark::CLTagHash> const & GetSpends() const;
    std::unordered_map<int, SparkCoinGroupInfo> const & GetCoinGroups() const;
    std::unordered_map<GroupElement, uint256, spark::CLTagHash> const & GetMempoolLTags() const;

    static CSparkState* GetState();

    std::size_t GetTotalCoins() const { return mintedCoins.size(); }

private:
    // Group Limit
    size_t maxCoinInGroup;
    size_t startGroupSize;

    // Latest anonymity set id;
    int latestCoinId;

    // Collection of coin groups. Map from id to LelantusCoinGroupInfo structure
    std::unordered_map<int, SparkCoinGroupInfo> coinGroups;

    // Set of all minted coins
    std::unordered_map<spark::Coin, CMintedCoinInfo, spark::CoinHash> mintedCoins;
    // Set of all used coin linking tags.
    std::unordered_map<GroupElement, int, spark::CLTagHash> usedLTags;

    typedef std::map<int, size_t> metainfo_container_t;
    metainfo_container_t extendedMintMetaInfo, mintMetaInfo, spendMetaInfo;
};

} // namespace spark

#endif //_MAIN_SPARK_STATE_H_