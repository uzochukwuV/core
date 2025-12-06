# Qubic Perpetual Futures (PERP) Contract Architecture

## Overview
A modular perpetual futures (perps) contract system similar to GMX, consisting of:
1. **PriceFeed Contract** - Manages oracle prices for multiple tokens
2. **Perpetuals Contract** - Handles trading, leverage, liquidations, and PnL

## System Architecture

### Supported Tokens (Phase 1)
- QUBIC (native, 1 QU = 1 unit)
- BTC (Bitcoin proxy)
- BNB (Binance Coin proxy)
- USDT (Stablecoin, 1 USDT ≈ 1 USD)

### Token IDs (modular design)
```cpp
enum TokenId {
    QUBIC_ID = 0,
    BTC_ID = 1,
    BNB_ID = 2,
    USDT_ID = 3
};
```

---

## 1. PRICE FEED CONTRACT (PriceFeed.h)

### Purpose
- Store current prices for all supported tokens
- Allow oracle/bot to update prices
- Provide price data to other contracts
- Track price history

### State Structure
```cpp
struct PriceFeed2 {};

struct PRICEFEED : public ContractBase {
    // Price storage: token_id -> current_price (scaled by 1e8)
    // Token prices stored as uint64, multiplied by 10^8 for precision

    // Example:
    // BTC = 45000 USD -> stored as 4500000000000
    // QUBIC = 0.10 USD -> stored as 10000000
};
```

### Key Functions

#### 1. updatePrice (PROCEDURE)
- **Input**: token_id, new_price, timestamp
- **Output**: success/failure
- **Access**: Only authorized oracle nodes
- **Purpose**: Update token price
- **Storage**: Store latest price per token + historical data

#### 2. getPrice (FUNCTION - read-only)
- **Input**: token_id
- **Output**: current_price, last_updated_timestamp
- **Purpose**: Retrieve current price for perp contract

#### 3. getPriceHistory (FUNCTION)
- **Input**: token_id, lookback_ticks
- **Output**: array of historical prices
- **Purpose**: Support liquidation and funding rate calculations

#### 4. setOracleAddress (PROCEDURE)
- **Input**: oracle_address
- **Output**: success
- **Purpose**: Update authorized oracle node

---

## 2. PERPETUALS CONTRACT (Perpetuals.h)

### Core Concepts

#### Position Structure
```cpp
struct Position {
    id trader;              // Trader address
    uint8 token_id;         // Which token (0-3)
    bool is_long;          // Long or short
    uint64 collateral;     // QU collateral amount
    uint64 size;           // Position size (notional value)
    uint64 entry_price;    // Entry price when opened
    uint64 leverage;       // Leverage used (1x to 10x, e.g., 500 = 5x)
    uint64 open_tick;      // Tick when position opened
    sint64 pnl;            // Current P&L (can be negative)
    uint64 funding_paid;   // Cumulative funding payments
};
```

#### Margin & Risk Management
- **Maintenance Margin**: 5% of position value (position liquidated if margin ratio falls below this)
- **Initial Margin**: 10% (minimum collateral to open)
- **Liquidation Price Calculation**:
  - Long: liquidation_price = entry_price × (1 - 1/leverage + 0.05)
  - Short: liquidation_price = entry_price × (1 + 1/leverage - 0.05)

#### Funding Rates
- **Purpose**: Incentivize market balance
- **Calculation**: Hourly based on open interest imbalance
- **Transfer**: Longs pay shorts when longs have more OI, vice versa

### State Structure
```cpp
struct PERPETUALS2 {};

struct PERPETUALS : public ContractBase {
    // Modular storage using HashMaps/Arrays:
    // - positions[trader_id] = array of positions
    // - token_data[token_id] = {total_long_oi, total_short_oi, cumulative_funding}
    // - balances[trader_id] = available QU balance
};
```

### Key Functions

#### Trading Functions

##### openPosition (PROCEDURE)
- **Input**: token_id, is_long, collateral, leverage
- **Output**: position_id, entry_price
- **Validations**:
  - Valid token_id
  - Leverage between 1x-10x
  - Collateral >= (collateral × leverage) × 10% (initial margin)
  - User has sufficient balance
- **Side Effects**:
  - Transfer collateral from user
  - Create position
  - Update open interest
  - Calculate and store entry price

##### closePosition (PROCEDURE)
- **Input**: position_id
- **Output**: realized_pnl, returned_collateral
- **Validations**:
  - Position exists
  - User owns position
- **Side Effects**:
  - Calculate exit price
  - Settle P&L
  - Reduce open interest
  - Return collateral + profit (or subtract loss)
  - Distribute funding payments

##### liquidatePosition (PROCEDURE)
- **Input**: position_id
- **Output**: liquidation_fee, liquidator_reward
- **Validations**:
  - Position is underwater (margin ratio < 5%)
  - Call permitted by anyone (public liquidation)
- **Side Effects**:
  - Force close position
  - Calculate liquidation fee (e.g., 5% of position)
  - Distribute fee to protocol/liquidator
  - Update open interest

#### Information Functions

##### getPosition (FUNCTION)
- **Input**: position_id or trader + index
- **Output**: Position struct with current P&L, margin ratio

##### getOpenInterest (FUNCTION)
- **Input**: token_id
- **Output**: total_long_oi, total_short_oi

##### getMarginRatio (FUNCTION)
- **Input**: position_id
- **Output**: margin_ratio_percent (10000 = 100%)

##### getPnL (FUNCTION)
- **Input**: position_id
- **Output**: unrealized_pnl (can be negative)

##### getFundingRate (FUNCTION)
- **Input**: token_id
- **Output**: current_hourly_funding_rate

#### Management Functions

##### settleHourly (PROCEDURE - system)
- Called each hour by system
- Distributes funding payments
- Updates cumulative funding
- Resets hourly metrics

##### emergencyWithdraw (PROCEDURE)
- Allows users to withdraw unused balance

---

## 3. Data Storage Design (Modular)

### Using QPI Collections

```cpp
// Token prices (PriceFeed)
QpiHashMap<uint8, uint64> current_prices;      // token_id -> price
QpiHashMap<uint8, uint64> last_update_time;    // token_id -> tick

// Positions (Perpetuals)
QpiArray<Position> all_positions;              // All positions
QpiHashMap<id, QpiArray<uint32>> user_positions; // trader -> position_ids

// Token statistics (Perpetuals)
struct TokenStats {
    uint64 total_long_oi;
    uint64 total_short_oi;
    sint64 cumulative_funding;  // Accumulated funding to distribute
};
QpiHashMap<uint8, TokenStats> token_stats;

// User balances
QpiHashMap<id, uint64> user_balances;          // Available QU balance
```

---

## 4. Operational Flow

### Opening a Position (Example)
```
User calls openPosition(BTC, true, 10000 QU collateral, 5x leverage)
↓
Contract validates:
  - User has 10000 QU available
  - Leverage is 5x (valid)
  - Collateral >= 10000 * 5 * 10% = 5000 QU ✓
↓
Query PriceFeed: BTC current price = 45000 USD
↓
Create Position:
  - size = 10000 * 5 = 50000 QU equivalent
  - entry_price = 45000
  - leverage = 5
↓
Update Statistics:
  - token_stats[BTC].total_long_oi += 50000
↓
Transfer collateral from user to contract
↓
Return position_id and entry_price
```

### Liquidation Check (Every Tick)
```
For each open position:
  Get current price from PriceFeed
  Calculate mark price P&L
  margin_ratio = (collateral + pnl) / (size * entry_price) * 100

  If margin_ratio < 5%:
    → Position is liquidatable
    → Anyone can call liquidatePosition()
    → Contract force-closes and distributes rewards
```

---

## 5. Funding Rate Mechanism

### Calculation
```
funding_rate_hourly = imbalance_ratio × max_rate
  where:
    imbalance_ratio = |long_oi - short_oi| / (long_oi + short_oi)
    max_rate = 0.1% per hour (0.001)
```

### Distribution
```
If long_oi > short_oi:
  Longs pay shorts:
    payment_per_long = (size / total_long_oi) × cumulative_funding
    shorts receive this

If short_oi > long_oi:
  Shorts pay longs:
    payment_per_short = (size / total_short_oi) × cumulative_funding
```

---

## 6. Fee Structure
- **Opening Fee**: 0.1% of position size
- **Closing Fee**: 0.1% of position size
- **Liquidation Fee**: 5% of position size
  - 50% to liquidator
  - 50% to protocol/DAO

---

## 7. Module Dependencies

```
PriceFeed.h
  ├─ Independent
  └─ Manages oracle prices

Perpetuals.h
  ├─ Depends on: PriceFeed (calls getPrice)
  └─ Core perps engine
```

---

## 8. Future Extensions
- Additional tokens (Add new token_id, update pricing)
- Dynamic leverage limits per token
- Structured orders (stop-loss, take-profit)
- Multi-collateral support (BTC, USDT as collateral)
- Advanced funding mechanisms (Twap-based, volatility-adjusted)

---

## 9. Node.js Bot Architecture

### Update Frequency
- Every 1-5 minutes
- Triggered by price change > 0.5%

### Data Sources
- CoinGecko API (free, reliable)
- Binance API (real-time, accurate)
- Custom aggregation logic

### Bot Functions
```javascript
{
  updatePrices(): void,           // Fetch latest prices
  submitToContract(): void,       // Call PriceFeed.updatePrice
  validatePrices(): boolean,      // Sanity checks
  handleErrors(): void            // Retry logic
}
```

