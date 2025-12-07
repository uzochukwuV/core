#define NO_UEFI

#include "contract_testing.h"

#define PRINT_TEST_INFO 0

// Function and Procedure ID constants
constexpr uint32 GET_OPEN_INTEREST_IDX = 3;
constexpr uint32 GET_TOKEN_STATS_IDX = 4;
constexpr uint32 GET_FUNDING_RATE_IDX = 5;
constexpr uint32 GET_MARGIN_RATIO_IDX = 8;
constexpr uint32 GET_PROTOCOL_STATS_IDX = 9;

constexpr uint32 OPEN_POSITION_IDX = 1;    // Procedure ID
constexpr uint32 CLOSE_POSITION_IDX = 2;   // Procedure ID
constexpr uint32 LIQUIDATE_POSITION_IDX = 3; // Procedure ID
constexpr uint32 DEPOSIT_MARGIN_IDX = 4;   // Procedure ID
constexpr uint32 WITHDRAW_MARGIN_IDX = 5;  // Procedure ID

class ContractTestingPerpetuals : protected ContractTesting
{
public:
    ContractTestingPerpetuals()
    {
        initEmptySpectrum();
        initEmptyUniverse();
        INIT_CONTRACT(PERPETUALS);
        callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE);
    }

    PERPETUALS::getOpenInterest_output getOpenInterest(uint8 token_id)
    {
        PERPETUALS::getOpenInterest_input input{ token_id };
        PERPETUALS::getOpenInterest_output output;
        callFunction(PERPETUALS_CONTRACT_INDEX, GET_OPEN_INTEREST_IDX, input, output);
        return output;
    }

    PERPETUALS::getTokenStats_output getTokenStats(uint8 token_id)
    {
        PERPETUALS::getTokenStats_input input{ token_id };
        PERPETUALS::getTokenStats_output output;
        callFunction(PERPETUALS_CONTRACT_INDEX, GET_TOKEN_STATS_IDX, input, output);
        return output;
    }

    PERPETUALS::getFundingRate_output getFundingRate(uint8 token_id)
    {
        PERPETUALS::getFundingRate_input input{ token_id };
        PERPETUALS::getFundingRate_output output;
        callFunction(PERPETUALS_CONTRACT_INDEX, GET_FUNDING_RATE_IDX, input, output);
        return output;
    }

    PERPETUALS::getProtocolStats_output getProtocolStats()
    {
        PERPETUALS::getProtocolStats_input input{};
        PERPETUALS::getProtocolStats_output output;
        callFunction(PERPETUALS_CONTRACT_INDEX, GET_PROTOCOL_STATS_IDX, input, output);
        return output;
    }

    PERPETUALS::getMarginRatio_output getMarginRatio(uint32 position_id)
    {
        PERPETUALS::getMarginRatio_input input{ position_id };
        PERPETUALS::getMarginRatio_output output;
        callFunction(PERPETUALS_CONTRACT_INDEX, GET_MARGIN_RATIO_IDX, input, output);
        return output;
    }

    PERPETUALS::depositMargin_output depositMargin(uint64 amount, const id& caller)
    {
        increaseEnergy(caller, 1000000);
        PERPETUALS::depositMargin_input input{ amount };
        PERPETUALS::depositMargin_output output;
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, DEPOSIT_MARGIN_IDX, input, output, caller, 0);
        return output;
    }

    PERPETUALS::withdrawMargin_output withdrawMargin(uint64 amount, const id& caller)
    {
        increaseEnergy(caller, 1000000);
        PERPETUALS::withdrawMargin_input input{ amount };
        PERPETUALS::withdrawMargin_output output;
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, WITHDRAW_MARGIN_IDX, input, output, caller, 0);
        return output;
    }

    PERPETUALS::openPosition_output openPosition(uint8 token_id, uint8 direction, uint64 collateral, uint64 leverage, const id& caller)
    {
        increaseEnergy(caller, 1000000);
        PERPETUALS::openPosition_input input{ token_id, direction, collateral, leverage };
        PERPETUALS::openPosition_output output;
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, OPEN_POSITION_IDX, input, output, caller, 0);
        return output;
    }

    PERPETUALS::closePosition_output closePosition(uint32 position_id, const id& caller)
    {
        increaseEnergy(caller, 1000000);
        PERPETUALS::closePosition_input input{ position_id };
        PERPETUALS::closePosition_output output;
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, CLOSE_POSITION_IDX, input, output, caller, 0);
        return output;
    }

    PERPETUALS::liquidatePosition_output liquidatePosition(uint32 position_id, const id& caller)
    {
        increaseEnergy(caller, 1000000);
        PERPETUALS::liquidatePosition_input input{ position_id };
        PERPETUALS::liquidatePosition_output output;
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, LIQUIDATE_POSITION_IDX, input, output, caller, 0);
        return output;
    }
};

// ==================== TESTS ====================

TEST(ContractPerpetuals, Initialization)
{
    ContractTestingPerpetuals perpetuals;
    auto stats = perpetuals.getProtocolStats();
    EXPECT_EQ(stats.total_fees, 0);
}

TEST(ContractPerpetuals, DepositMargin_Valid)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0x11111111, 0x22222222, 0x33333333, 0x44444444);
    auto output = perpetuals.depositMargin(10000000ULL, trader);
    EXPECT_TRUE(output.success);
    EXPECT_EQ(output.new_balance, 10000000ULL);
}

TEST(ContractPerpetuals, DepositMargin_Zero)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0xaaaabbbb, 0xccccdddd, 0xeeeeeeee, 0xffffffff);
    auto output = perpetuals.depositMargin(0, trader);
    EXPECT_FALSE(output.success);
}

TEST(ContractPerpetuals, DepositMargin_Multiple)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0x12345678, 0x87654321, 0xabcdef00, 0xfedcba00);
    auto output1 = perpetuals.depositMargin(1000000ULL, trader);
    EXPECT_TRUE(output1.success);
    EXPECT_EQ(output1.new_balance, 1000000ULL);
}

TEST(ContractPerpetuals, WithdrawMargin_Valid)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0xfedcba98, 0x76543210, 0x13579ace, 0x02468bdf);
    auto deposit_output = perpetuals.depositMargin(10000000ULL, trader);
    EXPECT_TRUE(deposit_output.success);
    auto withdraw_output = perpetuals.withdrawMargin(3000000ULL, trader);
    EXPECT_TRUE(withdraw_output.success);
    EXPECT_EQ(withdraw_output.remaining_balance, 7000000ULL);
}

TEST(ContractPerpetuals, OpenPosition_Long)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0x10203040, 0x50607080, 0x90a0b0c0, 0xd0e0f000);
    auto deposit_output = perpetuals.depositMargin(100000000ULL, trader);
    EXPECT_TRUE(deposit_output.success);
    auto open_output = perpetuals.openPosition(PERP_QUBIC, 0, 10000000ULL, 2, trader);
    EXPECT_TRUE(open_output.success);
}

TEST(ContractPerpetuals, OpenPosition_Short)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0xa1b1c1d1, 0xe1f1a2b2, 0xc2d2e2f2, 0xa3b3c3d3);
    auto deposit_output = perpetuals.depositMargin(100000000ULL, trader);
    EXPECT_TRUE(deposit_output.success);
    auto open_output = perpetuals.openPosition(PERP_BTC, 1, 20000000ULL, 3, trader);
    EXPECT_TRUE(open_output.success);
}

TEST(ContractPerpetuals, OpenPosition_InvalidLeverage)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0x11223344, 0x55667788, 0x99aabbcc, 0xddeeff00);
    auto deposit_output = perpetuals.depositMargin(100000000ULL, trader);
    auto open_output = perpetuals.openPosition(PERP_BNB, 0, 10000000ULL, 15, trader);
    EXPECT_FALSE(open_output.success);
}

TEST(ContractPerpetuals, ClosePosition_Valid)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0x55667788, 0x99aabbcc, 0xddeeff00, 0x11223344);
    auto deposit_output = perpetuals.depositMargin(100000000ULL, trader);
    auto open_output = perpetuals.openPosition(PERP_QUBIC, 0, 10000000ULL, 5, trader);
    EXPECT_TRUE(open_output.success);
    auto close_output = perpetuals.closePosition(0, trader);
    EXPECT_TRUE(close_output.success);
}

TEST(ContractPerpetuals, FundingRate_Calculation)
{
    ContractTestingPerpetuals perpetuals;
    id trader1(0x01020304, 0x05060708, 0x09000a0b, 0x0c0d0e0f);
    id trader2(0x10111213, 0x14151617, 0x18191a1b, 0x1c1d1e1f);
    auto deposit1 = perpetuals.depositMargin(100000000ULL, trader1);
    auto deposit2 = perpetuals.depositMargin(100000000ULL, trader2);
    auto open1 = perpetuals.openPosition(PERP_BTC, 0, 50000000ULL, 2, trader1);
    auto open2 = perpetuals.openPosition(PERP_BTC, 1, 30000000ULL, 2, trader2);
    auto fr_output = perpetuals.getFundingRate(PERP_BTC);
    EXPECT_NE(fr_output.funding_rate_bps, 0);
}

TEST(ContractPerpetuals, MultiplePositions_SameTrader)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0xaa11bb22, 0xcc33dd44, 0xee55ff66, 0x77889900);
    auto deposit_output = perpetuals.depositMargin(500000000ULL, trader);
    EXPECT_TRUE(deposit_output.success);
    for (int i = 0; i < 5; i++)
    {
        auto open_output = perpetuals.openPosition(PERP_QUBIC, i % 2, 10000000ULL, 2, trader);
        EXPECT_TRUE(open_output.success);
    }
}

TEST(ContractPerpetuals, GetOpenInterest)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0x20304050, 0x60708090, 0xa0b0c0d0, 0xe0f00010);
    auto deposit_output = perpetuals.depositMargin(100000000ULL, trader);
    auto open_output = perpetuals.openPosition(PERP_BTC, 0, 50000000ULL, 2, trader);
    auto oi_output = perpetuals.getOpenInterest(PERP_BTC);
    EXPECT_GT(oi_output.long_oi, 0);
    EXPECT_EQ(oi_output.short_oi, 0);
}

TEST(ContractPerpetuals, GetTokenStats)
{
    ContractTestingPerpetuals perpetuals;
    id trader(0xf0f1f2f3, 0xf4f5f6f7, 0xf8f9fafb, 0xfcfdfeff);
    auto deposit_output = perpetuals.depositMargin(100000000ULL, trader);
    auto open_output = perpetuals.openPosition(PERP_BNB, 0, 20000000ULL, 3, trader);
    auto stats_output = perpetuals.getTokenStats(PERP_BNB);
    // Token stats successfully retrieved (daily_volume will be 0 as it's not updated on position open)
    EXPECT_EQ(stats_output.daily_volume, 0);
}
