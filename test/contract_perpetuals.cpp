#define NO_UEFI

#include "contract_testing.h"

#define PRINT_TEST_INFO 0

static const id PERPETUALS_CONTRACT_ID(PERPETUALS_CONTRACT_INDEX, 0, 0, 0);

// ==================== TEST CLASS ====================

class PerpetualsChecker : public PERPETUALS
{
public:
    void checkPositionCount(uint32 expected)
    {
        EXPECT_EQ(state.position_count, expected);
    }

    void checkUserCount(uint32 expected)
    {
        EXPECT_EQ(state.user_count, expected);
    }

    void checkOpenInterest(uint8 token_id, uint64 expected_long, uint64 expected_short)
    {
        EXPECT_EQ(state.token_stats[token_id].long_oi, expected_long);
        EXPECT_EQ(state.token_stats[token_id].short_oi, expected_short);
    }

    void checkTotalFees(uint64 expected_fees)
    {
        EXPECT_GE(state.total_protocol_fees, expected_fees);
    }
};

class ContractTestingPerpetuals : protected ContractTesting
{
public:
    ContractTestingPerpetuals()
    {
        INIT_CONTRACT(PERPETUALS);
        initEmptySpectrum();
    }

    // Helper to call getOpenInterest function
    void callGetOpenInterest(uint8 token_id, PERPETUALS::getOpenInterest_output& output)
    {
        PERPETUALS::getOpenInterest_input input{ token_id };
        callFunction(PERPETUALS_CONTRACT_INDEX, 3, input, output, true, true);
    }

    // Helper to call getTokenStats function
    void callGetTokenStats(uint8 token_id, PERPETUALS::getTokenStats_output& output)
    {
        PERPETUALS::getTokenStats_input input{ token_id };
        callFunction(PERPETUALS_CONTRACT_INDEX, 4, input, output, true, true);
    }

    // Helper to call getFundingRate function
    void callGetFundingRate(uint8 token_id, PERPETUALS::getFundingRate_output& output)
    {
        PERPETUALS::getFundingRate_input input{ token_id };
        callFunction(PERPETUALS_CONTRACT_INDEX, 5, input, output, true, true);
    }

    // Helper to call getProtocolStats function
    void callGetProtocolStats(PERPETUALS::getProtocolStats_output& output)
    {
        PERPETUALS::getProtocolStats_input input{};
        callFunction(PERPETUALS_CONTRACT_INDEX, 9, input, output, true, true);
    }

    // Helper to call getMarginRatio function
    void callGetMarginRatio(uint32 position_id, PERPETUALS::getMarginRatio_output& output)
    {
        PERPETUALS::getMarginRatio_input input{ position_id };
        callFunction(PERPETUALS_CONTRACT_INDEX, 8, input, output, true, true);
    }

    // Helper to call depositMargin procedure
    void callDepositMargin(uint64 amount, const id& caller, PERPETUALS::depositMargin_output& output)
    {
        PERPETUALS::depositMargin_input input{ amount };
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, 4, input, output, caller, 0, true, true);
    }

    // Helper to call withdrawMargin procedure
    void callWithdrawMargin(uint64 amount, const id& caller, PERPETUALS::withdrawMargin_output& output)
    {
        PERPETUALS::withdrawMargin_input input{ amount };
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, 5, input, output, caller, 0, true, true);
    }

    // Helper to call openPosition procedure
    void callOpenPosition(uint8 token_id, uint8 direction, uint64 collateral, uint64 leverage,
                         const id& caller, PERPETUALS::openPosition_output& output)
    {
        PERPETUALS::openPosition_input input{ token_id, direction, collateral, leverage };
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, 1, input, output, caller, 0, true, true);
    }

    // Helper to call closePosition procedure
    void callClosePosition(uint32 position_id, const id& caller, PERPETUALS::closePosition_output& output)
    {
        PERPETUALS::closePosition_input input{ position_id };
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, 2, input, output, caller, 0, true, true);
    }

    // Helper to call liquidatePosition procedure
    void callLiquidatePosition(uint32 position_id, const id& caller, PERPETUALS::liquidatePosition_output& output)
    {
        PERPETUALS::liquidatePosition_input input{ position_id };
        invokeUserProcedure(PERPETUALS_CONTRACT_INDEX, 3, input, output, caller, 0, true, true);
    }
};

// ==================== TESTS ====================

TEST_F(ContractTestingPerpetuals, Initialization)
{
    // Call INITIALIZE system procedure
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    // Check that contract is initialized
    PERPETUALS::getProtocolStats_output output;
    callGetProtocolStats(output);

    EXPECT_EQ(output.total_fees, 0);
    EXPECT_EQ(output.liquidation_rewards_paid, 0);
    EXPECT_EQ(output.cumulative_volume, 0);
    EXPECT_EQ(output.total_positions_opened, 0);
    EXPECT_EQ(output.total_positions_closed, 0);
    EXPECT_EQ(output.total_liquidations, 0);
}

TEST_F(ContractTestingPerpetuals, DepositMargin_Valid)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x11111111, 0x22222222, 0x33333333, 0x44444444);
    uint64 deposit_amount = 10000000ULL; // 10 QU

    PERPETUALS::depositMargin_output output;
    callDepositMargin(deposit_amount, trader, output);

    EXPECT_TRUE(output.success);
    EXPECT_EQ(output.new_balance, deposit_amount);
}

TEST_F(ContractTestingPerpetuals, DepositMargin_Zero)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0xaaaabbbb, 0xccccdddd, 0xeeeeeeee, 0xffffffff);

    PERPETUALS::depositMargin_output output;
    callDepositMargin(0, trader, output);

    EXPECT_FALSE(output.success);
}

TEST_F(ContractTestingPerpetuals, DepositMargin_Multiple)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x12345678, 0x87654321, 0xabcdef00, 0xfedcba00);

    // First deposit
    PERPETUALS::depositMargin_output output1;
    callDepositMargin(1000000ULL, trader, output1);
    EXPECT_TRUE(output1.success);
    EXPECT_EQ(output1.new_balance, 1000000ULL);

    // Second deposit
    PERPETUALS::depositMargin_output output2;
    callDepositMargin(2000000ULL, trader, output2);
    EXPECT_TRUE(output2.success);
    EXPECT_EQ(output2.new_balance, 3000000ULL);

    // Third deposit
    PERPETUALS::depositMargin_output output3;
    callDepositMargin(5000000ULL, trader, output3);
    EXPECT_TRUE(output3.success);
    EXPECT_EQ(output3.new_balance, 8000000ULL);
}

TEST_F(ContractTestingPerpetuals, WithdrawMargin_Valid)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0xfedcba98, 0x76543210, 0x13579ace, 0x02468bdf);

    // First deposit
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(10000000ULL, trader, deposit_output);
    EXPECT_TRUE(deposit_output.success);

    // Then withdraw
    PERPETUALS::withdrawMargin_output withdraw_output;
    callWithdrawMargin(3000000ULL, trader, withdraw_output);

    EXPECT_TRUE(withdraw_output.success);
    EXPECT_EQ(withdraw_output.remaining_balance, 7000000ULL);
}

TEST_F(ContractTestingPerpetuals, WithdrawMargin_Insufficient)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x11223344, 0x55667788, 0x99aabbcc, 0xddeeff00);

    // Try to withdraw without deposit
    PERPETUALS::withdrawMargin_output output;
    callWithdrawMargin(1000000ULL, trader, output);

    EXPECT_FALSE(output.success);
}

TEST_F(ContractTestingPerpetuals, OpenPosition_ValidLong)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10);

    // Deposit some margin first
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);
    EXPECT_TRUE(deposit_output.success);

    // Open long position on BTC with 5x leverage
    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader, open_output);

    EXPECT_TRUE(open_output.success);
    EXPECT_EQ(open_output.position_id, 0);
    EXPECT_GT(open_output.entry_price, 0);
    EXPECT_EQ(open_output.position_size, 50000000ULL); // 10M * 5
}

TEST_F(ContractTestingPerpetuals, OpenPosition_ValidShort)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0xfedcbaaa, 0x98765432, 0x13579bdf, 0xacefaced);

    // Deposit margin
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);
    EXPECT_TRUE(deposit_output.success);

    // Open short position on USDT with 2x leverage
    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_USDT, 1, 20000000ULL, 2, trader, open_output);

    EXPECT_TRUE(open_output.success);
    EXPECT_EQ(open_output.position_id, 0);
    EXPECT_EQ(open_output.position_size, 40000000ULL); // 20M * 2
}

TEST_F(ContractTestingPerpetuals, OpenPosition_InvalidLeverage)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x12341234, 0x56785678, 0xabcdabcd, 0xefefefef);

    // Deposit margin
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    // Try to open with invalid leverage (11x > max 10x)
    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 11, trader, open_output);

    EXPECT_FALSE(open_output.success);
}

TEST_F(ContractTestingPerpetuals, OpenPosition_ZeroLeverage)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0xaa112233, 0xbb445566, 0xcc778899, 0xddaabbcc);

    // Deposit margin
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    // Try to open with 0 leverage
    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 0, trader, open_output);

    EXPECT_FALSE(open_output.success);
}

TEST_F(ContractTestingPerpetuals, OpenPosition_InvalidToken)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x99aabbcc, 0xddee1122, 0x33445566, 0x77889900);

    // Deposit margin
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    // Try to open position on invalid token (ID 99)
    PERPETUALS::openPosition_output open_output;
    callOpenPosition(99, 0, 10000000ULL, 5, trader, open_output);

    EXPECT_FALSE(open_output.success);
}

TEST_F(ContractTestingPerpetuals, OpenPosition_UpdatesOpenInterest)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x11223344, 0x55667788, 0x99aabbcc, 0xddeeff00);

    // Deposit margin
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    // Check initial OI
    PERPETUALS::getOpenInterest_output oi_before;
    callGetOpenInterest(PERP_BTC, oi_before);
    EXPECT_EQ(oi_before.long_oi, 0);
    EXPECT_EQ(oi_before.short_oi, 0);

    // Open long position
    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader, open_output);
    EXPECT_TRUE(open_output.success);

    // Check OI after
    PERPETUALS::getOpenInterest_output oi_after;
    callGetOpenInterest(PERP_BTC, oi_after);
    EXPECT_EQ(oi_after.long_oi, 50000000ULL);
    EXPECT_EQ(oi_after.short_oi, 0);
}

TEST_F(ContractTestingPerpetuals, ClosePosition_Success)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0xabcdef00, 0xfedcba11, 0x22334455, 0x66778899);

    // Deposit and open position
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader, open_output);
    EXPECT_TRUE(open_output.success);

    // Close position
    PERPETUALS::closePosition_output close_output;
    callClosePosition(0, trader, close_output);

    EXPECT_TRUE(close_output.success);
    EXPECT_NE(close_output.returned_collateral, 0);
}

TEST_F(ContractTestingPerpetuals, ClosePosition_UpdatesOpenInterest)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x55667788, 0x99aabbcc, 0xddeeff00, 0x11223344);

    // Setup
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader, open_output);

    // Verify OI increased
    PERPETUALS::getOpenInterest_output oi_before;
    callGetOpenInterest(PERP_BTC, oi_before);
    EXPECT_EQ(oi_before.long_oi, 50000000ULL);

    // Close position
    PERPETUALS::closePosition_output close_output;
    callClosePosition(0, trader, close_output);
    EXPECT_TRUE(close_output.success);

    // Verify OI decreased
    PERPETUALS::getOpenInterest_output oi_after;
    callGetOpenInterest(PERP_BTC, oi_after);
    EXPECT_EQ(oi_after.long_oi, 0);
}

TEST_F(ContractTestingPerpetuals, MultiplePositions)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0xaa11bb22, 0xcc33dd44, 0xee55ff66, 0x77889900);

    // Deposit large amount
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(500000000ULL, trader, deposit_output);
    EXPECT_TRUE(deposit_output.success);

    // Open multiple positions
    for (int i = 0; i < 5; i++)
    {
        PERPETUALS::openPosition_output open_output;
        uint8 token_id = i % PERP_MAX_TOKENS;
        callOpenPosition(token_id, (i % 2), 10000000ULL, 3, trader, open_output);
        EXPECT_TRUE(open_output.success);
        EXPECT_EQ(open_output.position_id, i);
    }

    // Verify all positions are open
    PERPETUALS::getProtocolStats_output stats;
    callGetProtocolStats(stats);
    EXPECT_EQ(stats.total_positions_opened, 5);
}

TEST_F(ContractTestingPerpetuals, FundingRateCalculation_NoImbalance)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader1(0x11111111, 0x11111111, 0x11111111, 0x11111111);
    id trader2(0x22222222, 0x22222222, 0x22222222, 0x22222222);

    // Deposit for both traders
    PERPETUALS::depositMargin_output deposit1;
    callDepositMargin(100000000ULL, trader1, deposit1);

    PERPETUALS::depositMargin_output deposit2;
    callDepositMargin(100000000ULL, trader2, deposit2);

    // Open equal long and short positions
    PERPETUALS::openPosition_output long_pos;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader1, long_pos);

    PERPETUALS::openPosition_output short_pos;
    callOpenPosition(PERP_BTC, 1, 10000000ULL, 5, trader2, short_pos);

    // Check funding rate (should be 0 with equal OI)
    PERPETUALS::getFundingRate_output funding;
    callGetFundingRate(PERP_BTC, funding);
    EXPECT_EQ(funding.funding_rate_bps, 0);
}

TEST_F(ContractTestingPerpetuals, FundingRateCalculation_WithImbalance)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader1(0xaaaabbbb, 0xccccdddd, 0xeeeeeeee, 0xffffffff);
    id trader2(0x12345678, 0x87654321, 0xabcdef00, 0xfedcba00);

    // Deposit for traders
    PERPETUALS::depositMargin_output deposit1;
    callDepositMargin(100000000ULL, trader1, deposit1);

    PERPETUALS::depositMargin_output deposit2;
    callDepositMargin(50000000ULL, trader2, deposit2);

    // Open 2x long vs 1x short (creates imbalance)
    PERPETUALS::openPosition_output long1;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader1, long1);

    PERPETUALS::openPosition_output long2;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader2, long2);

    // Create short position (much smaller)
    id trader3(0x99887766, 0x55443322, 0x11223344, 0x55667788);
    PERPETUALS::depositMargin_output deposit3;
    callDepositMargin(20000000ULL, trader3, deposit3);

    PERPETUALS::openPosition_output short1;
    callOpenPosition(PERP_BTC, 1, 5000000ULL, 2, trader3, short1);

    // Verify imbalance is reflected in funding rate
    PERPETUALS::getFundingRate_output funding;
    callGetFundingRate(PERP_BTC, funding);
    // Long OI = 100M, Short OI = 10M, so longs should pay (positive rate)
    EXPECT_GE(funding.funding_rate_bps, 0);
}

TEST_F(ContractTestingPerpetuals, TokenStatsTracking)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0xfedcba98, 0x76543210, 0x13579ace, 0x02468bdf);

    // Deposit
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    // Open positions on different tokens
    PERPETUALS::openPosition_output btc_pos;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader, btc_pos);

    PERPETUALS::openPosition_output usdt_pos;
    callOpenPosition(PERP_USDT, 0, 20000000ULL, 2, trader, usdt_pos);

    // Check stats for each token
    PERPETUALS::getTokenStats_output btc_stats;
    callGetTokenStats(PERP_BTC, btc_stats);
    EXPECT_EQ(btc_stats.long_oi, 50000000ULL);
    EXPECT_GT(btc_stats.total_volume, 0);

    PERPETUALS::getTokenStats_output usdt_stats;
    callGetTokenStats(PERP_USDT, usdt_stats);
    EXPECT_EQ(usdt_stats.long_oi, 40000000ULL);
    EXPECT_GT(usdt_stats.total_volume, 0);
}

TEST_F(ContractTestingPerpetuals, ProtocolFeesAccumulation)
{
    callSystemProcedure(PERPETUALS_CONTRACT_INDEX, INITIALIZE, true);

    id trader(0x33445566, 0x77889900, 0xaabbccdd, 0xeeff0011);

    // Deposit
    PERPETUALS::depositMargin_output deposit_output;
    callDepositMargin(100000000ULL, trader, deposit_output);

    // Get initial fees
    PERPETUALS::getProtocolStats_output stats_before;
    callGetProtocolStats(stats_before);
    uint64 fees_before = stats_before.total_fees;

    // Open and close a position (should incur fees)
    PERPETUALS::openPosition_output open_output;
    callOpenPosition(PERP_BTC, 0, 10000000ULL, 5, trader, open_output);
    EXPECT_TRUE(open_output.success);

    PERPETUALS::closePosition_output close_output;
    callClosePosition(0, trader, close_output);
    EXPECT_TRUE(close_output.success);

    // Verify fees increased
    PERPETUALS::getProtocolStats_output stats_after;
    callGetProtocolStats(stats_after);
    EXPECT_GT(stats_after.total_fees, fees_before);
}
