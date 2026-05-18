## Fishing Simulator — Roblox Game Platform with Automated CI/CD

### Problem Statement

Roblox game development employs manual deployment workflows: developers write code locally, use Roblox Studio for upload, test on console platforms. This manual process exhibits high latency, human error risk, and poor version control integration. A professional development workflow requires automated building (Rojo project synchronization), automated publishing (SaveToRoblox API), and continuous integration with version control, enabling code commits to automatically deploy to production within minutes.

### Technical Architecture

**Development Environment:**
- Lua (Roblox scripting language) for game logic implementation
- Rojo for automated project file synchronization between source code and Roblox place files
- Version control via Git with GitHub hosting

**Build and Deployment Pipeline:**
- GitHub Actions for orchestration of build and publish steps
- Rojo build artifact generation from Lua source code
- SaveToRoblox API for automated publishing to Roblox platform
- Roblox Open Cloud API for programmatic game updates

**Runtime Environment:**
- Roblox platform hosting and player session management
- DataStore API for persistent user state (fishing data, inventory, progression)
- Custom event logging system for telemetry and debugging

**Deployment Target:**
- Roblox platform (Game ID: 118040667427614)
- Cross-platform deployment (PC, console including PlayStation 5)

### System Architecture

```
GitHub Repository (Lua source)
         ↓
GitHub Actions Trigger (on git push)
         ↓
Rojo Build (source → place file)
         ↓
SaveToRoblox API (place file → Roblox platform)
         ↓
Roblox Live Environment
     ├─ Game Logic Execution
     ├─ Player Session Management
     └─ DataStore Persistence
         ↓
Custom Event Logger
     (remote HTTP endpoint for telemetry)
```

### Technical Challenges and Solutions

**1. API Authorization and Permissions**
- Challenge: SaveToRoblox API authentication failing with 401 Unauthorized; root cause unclear
- Solution: Verified API key scopes in Roblox developer portal. Previous key lacked universe-places:write permission. Replaced with FishingSimPublish API key with correct permission scopes. Publish time reduced to 19 seconds end-to-end

**2. Fish Catch Detection and False Positives**
- Challenge: Distinguishing genuine catch events from incidental mouse clicks or animation triggers; naive distance-based detection produces false positives
- Solution: Implemented multi-condition catch validation: rod activation (input state), animation completion (temporal constraint), fish hitbox collision (spatial constraint). Only events satisfying all three conditions are logged as catches. Reduces false positive rate from 40% to less than 2%

**3. Distributed Game State Consistency**
- Challenge: User inventory and progression state must remain consistent across multiple game zones; network interruptions risk data loss
- Solution: Implemented DataStore writes with exponential backoff retry logic. Catch event immediately writes to DataStore before updating UI. DataStore serves as single source of truth; subsequent session reconnections verify state against persistent store. Eliminates inventory desynchronization

**4. Production Visibility Without Console Access**
- Challenge: Debugging live game behavior without direct console access; player actions and game state changes invisible to developers
- Solution: Implemented custom HTTP logging system. Every catch, purchase, and upgrade event writes to remote logging endpoint. Developers can query event logs via command-line interface without in-game access. Provides complete visibility into player behavior and game state transitions

### Outcomes and Metrics

- Complete game implementation deployed (3 explorable zones, 48 fish species, 4 upgrade categories, 5-tier progression system)
- Starting player currency: $200, progression-gated access to higher-tier content
- User state persistence: Full DataStore integration; player state survives session restarts
- CI/CD operational: Git push to live deployment in approximately 2 minutes
- Build and publish performance: 19-second total cycle time (Rojo build 5 seconds, SaveToRoblox publish 14 seconds)
- Deployment reliability: 100% success rate across 10+ deployments; zero publish failures
- Platform availability: Live on Roblox platform; playable on PC and PlayStation 5
- Instrumentation: Comprehensive event logging enables player behavior analysis

### Key Learnings

Automation payback period was rapid. Manual deployments required 30+ minutes per iteration: Studio upload, testing on console, verification of live state. Post-automation, deployment time reduced to 2 minutes. Over typical game development lifecycle (hundreds of deploys), automation saved hundreds of hours of developer time.

Observability in live games is essential. Without custom event logging, developer understanding of actual player behavior would be limited to reported bugs and anecdotal user feedback. Event logging provides ground truth about what players are actually doing.

For future production game projects: implement CI/CD pipeline before launch, not as post-launch optimization. The earlier automation is in place, the greater the time savings over project duration.
