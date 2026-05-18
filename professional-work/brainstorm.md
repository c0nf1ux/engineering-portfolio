## Brainstorm — Trading Card Game Portfolio and Deckbuilding Platform

### Problem Statement

Magic: The Gathering players require tooling to efficiently manage card collections, search game data by multiple properties and constraints, and construct tournament-viable deck lists. Existing tools suffer from performance limitations, incomplete card databases, or insufficient integration with evolving metagame data. Brainstorm provides a unified platform for portfolio management, advanced card search with multiple constraint types, and real-time metagame analytics sourced from competitive tournament data.

### Technical Architecture

**Backend Infrastructure:**
- Node.js with Express framework for REST API design
- MongoDB for document storage of cards, user portfolios, and deck definitions
- Scheduled MTGGoldfish scraper for metagame trend analysis
- JWT-based authentication with encrypted session storage
- Render containerized hosting environment

**Frontend Application:**
- React 18 with Tailwind CSS for user interface
- React Router for client-side navigation and state management
- Vercel hosting with automatic deployment from Git
- Custom React hooks for domain-specific data management (card search, portfolio operations, deck building)

**Data Integration:**
- MTGGoldfish API integration for tournament metagame trends
- Automated scraper running on 6-hour schedule
- MongoDB caching with time-to-live semantics to balance freshness and API call reduction

### System Architecture

```
React Frontend
  ├─ Card Search Interface
  ├─ Portfolio Management
  └─ Deck Builder
         ↓
  REST API Layer
     ├─ Authentication (JWT)
     ├─ Card Search
     ├─ Portfolio CRUD
     └─ Deck Management
         ↓
  MongoDB
     ├─ Card Definitions (20,000+ records)
     ├─ User Profiles
     ├─ Deck Definitions
     └─ Portfolio State
         ↓
  Metagame Cache
     (6-hour refresh cycle)
```

### Technical Challenges and Solutions

**1. Card Database Scale and Search Performance**
- Challenge: MTG card database contains 20,000+ cards with multiple searchable attributes (name, cost, type, abilities, legality); naive search approaches exhibit performance degradation at scale
- Solution: Indexed MongoDB collection on frequently-searched attributes (card name, mana cost, card type). Search performance reduced from 500ms to under 100ms. Database queries are query-optimized with projection to return only necessary fields

**2. Portfolio State Computation Efficiency**
- Challenge: Portfolio calculations (aggregate deck statistics, card value summaries) are computationally expensive when computed on-demand for every view
- Solution: Denormalized portfolio view computed on write (when user updates deck or portfolio) rather than read. First-time computation cost is absorbed during infrequent mutation operations; subsequent reads retrieve pre-computed aggregates in under 50ms. Cache invalidation triggered on card price updates

**3. Full-Stack Data Flow Architecture**
- Challenge: Multiple asynchronous data flows between frontend (card search, portfolio view, deck building) and backend (API calls, database queries) create complex state management requirements and potential race conditions
- Solution: Isolated React hooks per domain (useCardSearch, usePortfolio, useDeckBuilder, useMeta) manage independent data flows and loading states. Each hook encapsulates API communication, error handling, and local state. Eliminates prop drilling and enables independent testing of data flow logic

### Outcomes and Metrics

- Five user-facing screens deployed (landing page, authentication, portfolio view, card search, deck builder, metagame view)
- Production bundle size: 84.4 KB (optimized)
- All screens connected to production API; zero mock data
- User acquisition: 12 beta testers from TCG community
- Test coverage: 100% integration testing (real API calls, no database mocks)
- Phase completion: Authentication, dashboard, deck builder, metagame scraper
- Post-beta roadmap: Stripe payment integration (Phase 3)

### Key Learnings

Mock data proved hazardous to code quality. Early development iterations relied on fake card collections and simulated portfolio data. These mocks masked integration bugs that appeared only when connecting to real APIs: race conditions in concurrent deck updates, missing error handling for network failures, data validation gaps. Real integration testing identified issues that unit tests against mocks never would.

Architectural lesson: Use real APIs from day one, even in development. The convenience of mocks is outweighed by the risk of building systems that function in development but fail in production.
