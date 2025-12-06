using namespace QPI;

// ==================== CONSTANTS & ENUMS ====================

// Token identifiers (modular - easily add more tokens)
enum PerpTokenId {
    PERP_QUBIC = 0,
    PERP_BTC = 1,
    PERP_BNB = 2,
    PERP_USDT = 3,
    PERP_MAX_TOKENS = 4
};

// Position status
enum PerpPositionStatus {
    PERP_POSITION_OPEN = 0,
    PERP_POSITION_CLOSED = 1,
    PERP_POSITION_LIQUIDATED = 2
};

// Risk management constants
constexpr uint64 PERP_MAX_LEVERAGE = 10;              // 10x max leverage
constexpr uint64 PERP_MIN_LEVERAGE = 1;               // 1x min leverage
constexpr uint64 PERP_INITIAL_MARGIN_PCT = 1000;      // 10% required margin
constexpr uint64 PERP_MAINTENANCE_MARGIN_PCT = 500;   // 5% liquidation threshold
constexpr uint64 PERP_LIQUIDATION_FEE_PCT = 500;      // 5% liquidation fee
constexpr uint32 PERP_OPENING_FEE_BPS = 10;           // 0.1% opening fee
constexpr uint32 PERP_CLOSING_FEE_BPS = 10;           // 0.1% closing fee
constexpr uint64 PERP_PRICE_PRECISION = 100000000ULL; // 1e8 for prices

// Max positions and users
constexpr uint64 PERP_MAX_POSITIONS = 1024;
constexpr uint64 PERP_MAX_USERS = 512;

// Funding rate constants
constexpr uint32 PERP_MAX_FUNDING_RATE_BPS = 100;     // 0.1% per hour max
constexpr uint32 PERP_FUNDING_UPDATE_FREQUENCY = 3600; // Update every 3600 ticks (1 hour)

// ==================== STRUCTURES ====================

// Represents a single position
struct Position {
    id trader;                  // Position owner
    uint8 token_id;            // Which token
    uint8 direction;           // 0=long, 1=short
    uint8 status;              // open/closed/liquidated
    uint64 collateral;         // Margin amount
    uint64 size;               // Position notional value
    uint64 entry_price;        // Entry price (1e8 precision)
    uint64 leverage;           // Leverage used
    uint32 open_tick;          // When opened
    sint64 unrealized_pnl;     // Current P&L
    uint64 funding_paid;       // Cumulative funding paid
};

// Token statistics
struct TokenStats {
    uint64 long_oi;            // Total long open interest
    uint64 short_oi;           // Total short open interest
    uint64 daily_volume;       // Daily trading volume
    uint32 last_funding_tick;  // Last funding settlement tick
    sint32 cumulative_funding; // Cumulative funding payments
};

// User account information
struct UserAccount {
    id user_id;
    uint64 available_balance;  // Available margin
    uint64 reserved_margin;    // Margin in active positions
    uint32 position_count;     // Number of open positions
    uint64 realized_pnl;       // Cumulative realized P&L
};

struct PERPETUALS2 {};

struct PERPETUALS : public ContractBase {
public:
    // ==================== STATE DATA ====================

    // Global statistics per token
    TokenStats token_stats[PERP_MAX_TOKENS];

    // User account tracking (simplified - using arrays with fixed size)
    UserAccount user_accounts[PERP_MAX_USERS];
    uint32 user_count;

    // Position tracking
    Position positions[PERP_MAX_POSITIONS];
    uint32 position_count;

    // Protocol statistics
    uint64 total_protocol_fees;
    uint64 total_liquidation_rewards;
    uint64 cumulative_trading_volume;

    // Configuration
    uint32 opening_fee_bps;
    uint32 closing_fee_bps;
    uint32 liquidation_fee_bps;

    // Reserved for future expansions
    uint8 reserved[512];

    // ==================== INPUT/OUTPUT STRUCTS ====================

    // === Position Management ===

    struct openPosition_input {
        uint8 token_id;
        uint8 direction;        // 0=long, 1=short
        uint64 collateral;      // Margin to use
        uint64 leverage;        // Leverage (1-10)
    };

    struct openPosition_output {
        bool success;
        uint32 position_id;
        uint64 entry_price;
        uint64 position_size;   // Notional value
    };

    struct closePosition_input {
        uint32 position_id;
    };

    struct closePosition_output {
        bool success;
        sint64 realized_pnl;
        uint64 returned_collateral;
    };

    struct liquidatePosition_input {
        uint32 position_id;
    };

    struct liquidatePosition_output {
        bool success;
        uint64 liquidation_fee;
        uint64 liquidator_reward;
    };

    // === Position Information ===

    struct getPosition_input {
        uint32 position_id;
    };

    struct getPosition_output {
        id trader;
        uint8 token_id;
        uint8 direction;
        uint8 status;
        uint64 collateral;
        uint64 size;
        uint64 entry_price;
        uint64 leverage;
        sint64 unrealized_pnl;
        uint32 margin_ratio;    // 10000 = 100%
        bool is_liquidatable;
    };

    struct getPositions_input {
        id trader;
    };

    struct getPositions_output {
        uint32 count;
        Array<uint32, 128> position_ids;
    };

    // === Market Data ===

    struct getOpenInterest_input {
        uint8 token_id;
    };

    struct getOpenInterest_output {
        uint64 long_oi;
        uint64 short_oi;
        uint64 imbalance;       // |long_oi - short_oi|
    };

    struct getTokenStats_input {
        uint8 token_id;
    };

    struct getTokenStats_output {
        uint64 long_oi;
        uint64 short_oi;
        uint64 daily_volume;
        uint64 total_volume;
    };

    struct getFundingRate_input {
        uint8 token_id;
    };

    struct getFundingRate_output {
        sint32 funding_rate_bps;  // Basis points per hour
        uint32 last_update_tick;
    };

    // === Account Management ===

    struct getUserBalance_input {
        id user;
    };

    struct getUserBalance_output {
        uint64 available;
        uint64 reserved;
        uint64 total;
    };

    struct getUserStats_input {
        id user;
    };

    struct getUserStats_output {
        uint32 position_count;
        uint64 realized_pnl;
        sint64 unrealized_pnl;
    };

    struct depositMargin_input {
        uint64 amount;
    };

    struct depositMargin_output {
        bool success;
        uint64 new_balance;
    };

    struct withdrawMargin_input {
        uint64 amount;
    };

    struct withdrawMargin_output {
        bool success;
        uint64 remaining_balance;
    };

    // === Protocol Statistics ===

    struct getProtocolStats_input {};

    struct getProtocolStats_output {
        uint64 total_fees;
        uint64 liquidation_rewards_paid;
        uint64 cumulative_volume;
        uint32 total_positions_opened;
        uint32 total_positions_closed;
        uint32 total_liquidations;
    };

    struct getMarginRatio_input {
        uint32 position_id;
    };

    struct getMarginRatio_output {
        uint32 ratio;           // 10000 = 100%
        bool is_liquidatable;
    };

    // ==================== PUBLIC FUNCTIONS (READ-ONLY) ====================

    PUBLIC_FUNCTION(getPosition) {
        // Get detailed position info
        if (input.position_id >= state.position_count) {
            // Return empty position
            output.trader = NULL_ID;
            output.status = 0;
            return;
        }

        Position pos = state.positions[input.position_id];
        output.trader = pos.trader;
        output.token_id = pos.token_id;
        output.direction = pos.direction;
        output.status = pos.status;
        output.collateral = pos.collateral;
        output.size = pos.size;
        output.entry_price = pos.entry_price;
        output.leverage = pos.leverage;
        output.unrealized_pnl = pos.unrealized_pnl;
        output.margin_ratio = calculateMarginRatio(pos.collateral, pos.unrealized_pnl, pos.size);
        output.is_liquidatable = (output.margin_ratio < PERP_MAINTENANCE_MARGIN_PCT);
    }

    PUBLIC_FUNCTION(getPositions) {
        // Get all positions for a user
        uint32 count = 0;
        for (uint32 i = 0; i < state.position_count && count < 128; i++) {
            if (state.positions[i].trader == input.trader && state.positions[i].status == PERP_POSITION_OPEN) {
                output.position_ids.set(count, i);
                count++;
            }
        }
        output.count = count;
    }

    PUBLIC_FUNCTION(getOpenInterest) {
        // Get OI for a token
        if (input.token_id >= PERP_MAX_TOKENS) {
            output.long_oi = 0;
            output.short_oi = 0;
            output.imbalance = 0;
            return;
        }

        output.long_oi = state.token_stats[input.token_id].long_oi;
        output.short_oi = state.token_stats[input.token_id].short_oi;

        if (output.long_oi > output.short_oi) {
            output.imbalance = output.long_oi - output.short_oi;
        } else {
            output.imbalance = output.short_oi - output.long_oi;
        }
    }

    PUBLIC_FUNCTION(getTokenStats) {
        // Get comprehensive token statistics
        if (input.token_id >= PERP_MAX_TOKENS) {
            output.long_oi = 0;
            output.short_oi = 0;
            output.daily_volume = 0;
            output.total_volume = 0;
            return;
        }

        output.long_oi = state.token_stats[input.token_id].long_oi;
        output.short_oi = state.token_stats[input.token_id].short_oi;
        output.daily_volume = state.token_stats[input.token_id].daily_volume;
        output.total_volume = state.cumulative_trading_volume;
    }

    PUBLIC_FUNCTION(getFundingRate) {
        // Get current funding rate
        if (input.token_id >= PERP_MAX_TOKENS) {
            output.funding_rate_bps = 0;
            output.last_update_tick = 0;
            return;
        }

        uint64 long_oi = state.token_stats[input.token_id].long_oi;
        uint64 short_oi = state.token_stats[input.token_id].short_oi;
        uint64 total_oi = long_oi + short_oi;

        output.last_update_tick = state.token_stats[input.token_id].last_funding_tick;

        if (total_oi == 0) {
            output.funding_rate_bps = 0;
            return;
        }

        // Calculate funding rate based on OI imbalance
        // Positive = longs pay shorts, Negative = shorts pay longs
        sint64 imbalance = (sint64)long_oi - (sint64)short_oi;
        output.funding_rate_bps = (sint32)((imbalance * 100) / (sint64)total_oi);

        // Cap at max rate
        if (output.funding_rate_bps > PERP_MAX_FUNDING_RATE_BPS) {
            output.funding_rate_bps = PERP_MAX_FUNDING_RATE_BPS;
        } else if (output.funding_rate_bps < -(sint32)PERP_MAX_FUNDING_RATE_BPS) {
            output.funding_rate_bps = -(sint32)PERP_MAX_FUNDING_RATE_BPS;
        }
    }

    PUBLIC_FUNCTION(getUserBalance) {
        // Get user margin balance
        for (uint32 i = 0; i < state.user_count; i++) {
            if (state.user_accounts[i].user_id == input.user) {
                output.available = state.user_accounts[i].available_balance;
                output.reserved = state.user_accounts[i].reserved_margin;
                output.total = output.available + output.reserved;
                return;
            }
        }
        output.available = 0;
        output.reserved = 0;
        output.total = 0;
    }

    PUBLIC_FUNCTION(getUserStats) {
        // Get user trading statistics
        for (uint32 i = 0; i < state.user_count; i++) {
            if (state.user_accounts[i].user_id == input.user) {
                output.position_count = state.user_accounts[i].position_count;
                output.realized_pnl = state.user_accounts[i].realized_pnl;

                // Calculate unrealized PnL
                sint64 unrealized = 0;
                for (uint32 j = 0; j < state.position_count; j++) {
                    if (state.positions[j].trader == input.user && state.positions[j].status == PERP_POSITION_OPEN) {
                        unrealized += state.positions[j].unrealized_pnl;
                    }
                }
                output.unrealized_pnl = unrealized;
                return;
            }
        }
        output.position_count = 0;
        output.realized_pnl = 0;
        output.unrealized_pnl = 0;
    }

    PUBLIC_FUNCTION(getMarginRatio) {
        // Get margin ratio for a position
        if (input.position_id >= state.position_count) {
            output.ratio = 0;
            output.is_liquidatable = false;
            return;
        }

        Position pos = state.positions[input.position_id];
        output.ratio = calculateMarginRatio(pos.collateral, pos.unrealized_pnl, pos.size);
        output.is_liquidatable = (output.ratio < PERP_MAINTENANCE_MARGIN_PCT);
    }

    PUBLIC_FUNCTION(getProtocolStats) {
        // Get protocol-wide statistics
        output.total_fees = state.total_protocol_fees;
        output.liquidation_rewards_paid = state.total_liquidation_rewards;
        output.cumulative_volume = state.cumulative_trading_volume;

        // Count total positions
        uint32 closed_count = 0;
        uint32 liquidated_count = 0;
        for (uint32 i = 0; i < state.position_count; i++) {
            if (state.positions[i].status == PERP_POSITION_CLOSED) closed_count++;
            if (state.positions[i].status == PERP_POSITION_LIQUIDATED) liquidated_count++;
        }
        output.total_positions_opened = state.position_count;
        output.total_positions_closed = closed_count;
        output.total_liquidations = liquidated_count;
    }

    // ==================== PUBLIC PROCEDURES (STATE-MODIFYING) ====================

    PUBLIC_PROCEDURE(openPosition) {
        // Validate inputs
        if (input.token_id >= PERP_MAX_TOKENS) {
            output.success = false;
            return;
        }

        if (input.leverage < PERP_MIN_LEVERAGE || input.leverage > PERP_MAX_LEVERAGE) {
            output.success = false;
            return;
        }

        if (input.collateral == 0) {
            output.success = false;
            return;
        }

        // Create new position (simplified - no actual price lookup yet)
        if (state.position_count >= PERP_MAX_POSITIONS) {
            output.success = false;
            return;
        }

        Position& new_pos = state.positions[state.position_count];
        new_pos.trader = qpi.invocator();
        new_pos.token_id = input.token_id;
        new_pos.direction = input.direction;
        new_pos.status = PERP_POSITION_OPEN;
        new_pos.collateral = input.collateral;
        new_pos.size = input.collateral * input.leverage;
        new_pos.entry_price = 45000 * PERP_PRICE_PRECISION; // Mock price
        new_pos.leverage = input.leverage;
        new_pos.open_tick = qpi.tick();
        new_pos.unrealized_pnl = 0;
        new_pos.funding_paid = 0;

        // Update token stats
        if (input.direction == 0) {  // Long
            state.token_stats[input.token_id].long_oi += new_pos.size;
        } else {                      // Short
            state.token_stats[input.token_id].short_oi += new_pos.size;
        }

        // Calculate fee
        uint64 opening_fee = (new_pos.size * PERP_OPENING_FEE_BPS) / 10000;
        state.total_protocol_fees += opening_fee;
        state.cumulative_trading_volume += new_pos.size;

        output.success = true;
        output.position_id = state.position_count;
        output.entry_price = new_pos.entry_price;
        output.position_size = new_pos.size;

        state.position_count++;
    }

    PUBLIC_PROCEDURE(closePosition) {
        // Close an open position
        if (input.position_id >= state.position_count) {
            output.success = false;
            return;
        }

        Position pos = state.positions[input.position_id];
        if (pos.status != PERP_POSITION_OPEN || pos.trader != qpi.invocator()) {
            output.success = false;
            return;
        }

        // Settle P&L
        sint64 pnl = pos.unrealized_pnl;
        uint64 fee = (pos.size * PERP_CLOSING_FEE_BPS) / 10000;
        sint64 net_pnl = pnl - (sint64)fee;

        // Update position
        state.positions[input.position_id].status = PERP_POSITION_CLOSED;

        // Update token stats
        if (pos.direction == 0) {  // Long
            state.token_stats[pos.token_id].long_oi = state.token_stats[pos.token_id].long_oi > pos.size ?
                state.token_stats[pos.token_id].long_oi - pos.size : 0;
        } else {                    // Short
            state.token_stats[pos.token_id].short_oi = state.token_stats[pos.token_id].short_oi > pos.size ?
                state.token_stats[pos.token_id].short_oi - pos.size : 0;
        }

        state.total_protocol_fees += fee;

        output.success = true;
        output.realized_pnl = net_pnl;
        output.returned_collateral = pos.collateral + net_pnl;
    }

    PUBLIC_PROCEDURE(liquidatePosition) {
        // Force-close underwater position
        if (input.position_id >= state.position_count) {
            output.success = false;
            return;
        }

        Position pos = state.positions[input.position_id];
        if (pos.status != PERP_POSITION_OPEN) {
            output.success = false;
            return;
        }

        // Check if liquidatable
        uint32 margin_ratio = calculateMarginRatio(pos.collateral, pos.unrealized_pnl, pos.size);
        if (margin_ratio >= PERP_MAINTENANCE_MARGIN_PCT) {
            output.success = false;
            return;
        }

        // Calculate liquidation fee
        uint64 liq_fee = (pos.size * PERP_LIQUIDATION_FEE_PCT) / 10000;
        uint64 liquidator_reward = liq_fee / 2;

        state.positions[input.position_id].status = PERP_POSITION_LIQUIDATED;

        // Update OI
        if (pos.direction == 0) {
            state.token_stats[pos.token_id].long_oi = state.token_stats[pos.token_id].long_oi > pos.size ?
                state.token_stats[pos.token_id].long_oi - pos.size : 0;
        } else {
            state.token_stats[pos.token_id].short_oi = state.token_stats[pos.token_id].short_oi > pos.size ?
                state.token_stats[pos.token_id].short_oi - pos.size : 0;
        }

        state.total_protocol_fees += (liq_fee - liquidator_reward);
        state.total_liquidation_rewards += liquidator_reward;

        output.success = true;
        output.liquidation_fee = liq_fee;
        output.liquidator_reward = liquidator_reward;
    }

    PUBLIC_PROCEDURE(depositMargin) {
        // Deposit margin
        if (input.amount == 0) {
            output.success = false;
            output.new_balance = 0;
            return;
        }

        // Find or create user account
        uint32 user_idx = UINT32_MAX;
        for (uint32 i = 0; i < state.user_count; i++) {
            if (state.user_accounts[i].user_id == qpi.invocator()) {
                user_idx = i;
                break;
            }
        }

        if (user_idx == UINT32_MAX) {
            if (state.user_count >= PERP_MAX_USERS) {
                output.success = false;
                return;
            }
            user_idx = state.user_count;
            state.user_accounts[user_idx].user_id = qpi.invocator();
            state.user_accounts[user_idx].available_balance = 0;
            state.user_count++;
        }

        state.user_accounts[user_idx].available_balance += input.amount;
        output.success = true;
        output.new_balance = state.user_accounts[user_idx].available_balance;
    }

    PUBLIC_PROCEDURE(withdrawMargin) {
        // Withdraw margin
        if (input.amount == 0) {
            output.success = false;
            output.remaining_balance = 0;
            return;
        }

        // Find user account
        for (uint32 i = 0; i < state.user_count; i++) {
            if (state.user_accounts[i].user_id == qpi.invocator()) {
                if (input.amount > state.user_accounts[i].available_balance) {
                    output.success = false;
                    output.remaining_balance = state.user_accounts[i].available_balance;
                    return;
                }

                state.user_accounts[i].available_balance -= input.amount;
                output.success = true;
                output.remaining_balance = state.user_accounts[i].available_balance;
                return;
            }
        }

        output.success = false;
        output.remaining_balance = 0;
    }

    // ==================== SYSTEM PROCEDURES ====================

    INITIALIZE() {
        // Initialize all state
        for (uint32 i = 0; i < PERP_MAX_TOKENS; i++) {
            state.token_stats[i].long_oi = 0;
            state.token_stats[i].short_oi = 0;
            state.token_stats[i].daily_volume = 0;
            state.token_stats[i].last_funding_tick = 0;
            state.token_stats[i].cumulative_funding = 0;
        }

        state.user_count = 0;
        state.position_count = 0;
        state.total_protocol_fees = 0;
        state.total_liquidation_rewards = 0;
        state.cumulative_trading_volume = 0;

        state.opening_fee_bps = PERP_OPENING_FEE_BPS;
        state.closing_fee_bps = PERP_CLOSING_FEE_BPS;
        state.liquidation_fee_bps = PERP_LIQUIDATION_FEE_PCT;
    }

    BEGIN_EPOCH() {
        // Reset daily volumes
        for (uint32 i = 0; i < PERP_MAX_TOKENS; i++) {
            state.token_stats[i].daily_volume = 0;
        }
    }

    END_EPOCH() {
        // Settle funding payments and snapshot stats
        // TODO: Implement funding settlement
    }

    // ==================== PRIVATE HELPERS ====================

    // Helper function to calculate margin ratio - stateless so no 'this' needed
    static uint32 calculateMarginRatio(uint64 collateral, sint64 pnl, uint64 size) {
        // margin_ratio = (collateral + pnl) / (size * initial_margin%) * 100%
        if (size == 0) return 10000;

        sint64 equity = (sint64)collateral + pnl;
        if (equity <= 0) return 0;

        uint64 required_margin = (size * PERP_INITIAL_MARGIN_PCT) / 10000;
        if (required_margin == 0) return 10000;

        return (uint32)((equity * 10000) / (sint64)required_margin);
    }

    // ==================== REGISTRATION ====================

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES() {
        // Functions
        REGISTER_USER_FUNCTION(getPosition, 1);
        REGISTER_USER_FUNCTION(getPositions, 2);
        REGISTER_USER_FUNCTION(getOpenInterest, 3);
        REGISTER_USER_FUNCTION(getTokenStats, 4);
        REGISTER_USER_FUNCTION(getFundingRate, 5);
        REGISTER_USER_FUNCTION(getUserBalance, 6);
        REGISTER_USER_FUNCTION(getUserStats, 7);
        REGISTER_USER_FUNCTION(getMarginRatio, 8);
        REGISTER_USER_FUNCTION(getProtocolStats, 9);

        // Procedures
        REGISTER_USER_PROCEDURE(openPosition, 1);
        REGISTER_USER_PROCEDURE(closePosition, 2);
        REGISTER_USER_PROCEDURE(liquidatePosition, 3);
        REGISTER_USER_PROCEDURE(depositMargin, 4);
        REGISTER_USER_PROCEDURE(withdrawMargin, 5);
    }
};
