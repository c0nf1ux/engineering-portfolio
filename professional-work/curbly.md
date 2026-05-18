## Curbly — On-Demand Trash Pickup Platform

### Problem Statement

Apartment dwellers in multi-unit residential buildings encounter a recurring operational inefficiency: trash compactor facilities are typically located in basement or remote areas, necessitating multiple weekly trips by residents. This friction point is particularly acute for time-constrained professionals. Curbly addresses this market gap by providing on-demand daily trash pickup service wherein trained haulers collect waste directly from apartment units, eliminating the need for residents to transport refuse to centralized facilities.

### Technical Architecture

**Backend Infrastructure:**
- Node.js with Express framework for API layer
- MongoDB Atlas for persistent data storage
- Socket.io for real-time bidirectional communication and GPS tracking
- JWT-based authentication with encrypted session management
- Stripe API integration for payment processing and KYC compliance

**Frontend Application:**
- React with Tailwind CSS for customer and hauler interfaces
- Vercel hosting for content delivery and serverless functions
- Real-time location tracking visualization

**Deployment Environment:**
- Frontend: Vercel (CDN with automatic scaling)
- Backend: Render (containerized Node.js service)
- Database: MongoDB Atlas (managed service with replication)

### System Design

```
Customer Application → REST API Layer → MongoDB (user profiles, orders, GPS coordinates)
Hauler Application   → WebSocket Layer → Socket.io Real-time Updates
                     → Stripe Payment Processing
```

### Technical Challenges and Solutions

**1. Real-Time GPS Tracking at Scale**
- Challenge: Continuous GPS position updates from haulers create excessive database write load, potentially causing performance degradation
- Solution: Implemented Socket.io pub/sub pattern for client-side streaming without persisting every update. Only start/end coordinates and aggregate route data are committed to database, reducing write operations by 95% while maintaining user visibility

**2. Waitlist Management and Incentive Allocation**
- Challenge: Equitable distribution of early adopter incentives across geographically dispersed user base
- Solution: Developed 3-tier automatic assignment system tracking via JWT claims and order history. Early customers (first cohort) receive tiered discount structure and referral reward mechanisms

**3. Infrastructure Cost Optimization**
- Challenge: MongoDB cluster costs escalated from baseline to $255/month during 4-day testing period, threatening operational sustainability
- Solution: Identified and deleted duplicate M30 cluster. Implemented strategic database indexing on frequently-queried user attributes. Validated that single M10 cluster handles MVP workload. Stabilized recurring cost at $60/month

### Outcomes and Metrics

- MVP deployed and tested in production environment (May 2026)
- Validation testing scheduled with target user demographics (Foley, AL and Gulf Shores region)
- Dual-application architecture complete (customer and hauler interfaces)
- End-to-end GPS integration operational
- Authentication security hardened to production standards
- Phase 3 requirements (Stripe KYC integration) scoped for post-beta deployment
- Infrastructure costs stabilized and sustainable

### Key Learnings

Early-stage cost monitoring proved critical to project viability. The infrastructure cost spike remained undetected until manual audit identified duplicate resources. Current practice includes real-time cost monitoring on all backend services from initial deployment. Additionally, streaming real-time data (Socket.io GPS updates) requires deliberate database architecture planning; naive persistence of every update creates scaling bottlenecks that become expensive to remediate post-deployment.

For future projects deploying real-time features, pre-design data persistence strategy to distinguish between ephemeral streaming data and authoritative persistent state.
