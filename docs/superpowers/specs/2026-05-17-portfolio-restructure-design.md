# Portfolio Restructuring Design — 2026-05-17

**Goal:** Restructure CS portfolio to showcase professional work as primary focus, with formal coursework as supporting education reference. Target audience: hiring managers.

**Status:** Design approved, ready for implementation plan.

---

## Executive Summary

Current state: Portfolio emphasizes coursework (10 SNHU courses, 2,400+ files). Real work (5 shipped products) is invisible to recruiters.

Target state: Professional work is the headline. Coursework remains but moves to secondary position as education validation.

**Key insight:** Hiring managers care about shipping ability, systems thinking, and problem-solving. Lead with that.

---

## Professional Projects (5 Total)

### Project Roster

| Order | Repo | Status | Domain | Focus |
|-------|------|--------|--------|-------|
| 1 | `curbly` | Deployed | Gig logistics | Real product, user-facing, revenue model |
| 2 | `walter` | Live trading | Autonomous systems | Data pipelines, self-tuning agents |
| 3 | `tcinvestments-hub` (Brainstorm) | Deployed | TCG platform | Full-stack, MongoDB, backend+frontend |
| 4 | `finch-prey-simulator` | Hardware | Robotics/AI | Behavioral systems, real hardware |
| 5 | `fishing-simulator` | Live on Roblox | Game automation | Creative shipping, live platform |

### Case Study Structure (per project)

Each project includes:

1. **Problem Statement** — What problem does it solve? Why does it matter?
2. **Tech Stack** — Languages, frameworks, databases, platforms, deployment targets
3. **Architecture Overview** — Key components, data flow, system diagram (Mermaid)
4. **Key Challenges & Solutions** — What was hard? How did you solve it? (2-3 key challenges)
5. **Outcomes & Metrics** — Users? Revenue? Performance? Learning outcomes?
6. **Reflections & Learnings** — What you'd do differently, lessons for future projects

### Ordering Rationale

**Curbly first:** Real marketplace product (problem: daily trash pickup needs for apartments, solution: on-demand hauling). Proves shipping ability.

**Walter second:** Autonomous trading with data pipelines and self-tuning parameters. Proves systems thinking and algorithmic sophistication.

**Brainstorm third:** Full-stack TCG platform (backend API, MongoDB, React frontend). Proves full-stack competency and architectural depth.

**Finch fourth:** Hardware robotics with behavioral AI (prey simulation, state machines, real BLE integration). Proves technical versatility and ability to work with constraints.

**Fishing Simulator fifth:** Creative automation on Roblox, fully deployed live game. Proves shipping for fun and initiative.

**Story arc:** "Here's what I ship when it matters → here's my systems thinking → here's my full-stack depth → here's my hardware expertise → here's what I ship for passion."

---

## New File Structure

```
cs-portfolio/
├── README.md (RESTRUCTURED)
│   ├── Header + intro
│   ├── PROFESSIONAL WORK (primary)
│   │   ├── Overview paragraph
│   │   ├── 5 project case studies (inline or linked)
│   │   └── Tech stack summary
│   ├── FORMAL EDUCATION (secondary)
│   │   ├── Brief summary
│   │   ├── Domain breakdown
│   │   └── Link to detailed courses/
│   └── Contact/Links
│
├── professional-work/ (NEW)
│   ├── curbly.md (detailed case study)
│   ├── walter.md (detailed case study)
│   ├── brainstorm.md (detailed case study)
│   ├── finch.md (detailed case study)
│   └── fishing-simulator.md (detailed case study)
│
├── courses/ (existing, unchanged)
│   ├── CS-230_Operating-Platforms/
│   ├── CS-250_Software-Development-Lifecycle/
│   ├── ... (10 courses total)
│
├── mathematics/ (existing, unchanged)
├── other-courses/ (existing, unchanged)
├── projects/ (existing, unchanged - legacy OpenGL portfolio)
├── archive/ (existing, unchanged)
├── misc/ (existing, unchanged)
│
└── docs/superpowers/specs/ (NEW)
    └── 2026-05-17-portfolio-restructure-design.md (this file)
```

---

## README.md Template Structure

### Header
- GitHub profile link
- Name + title
- Brief tagline ("Shipped 5 products across trading, logistics, gaming, robotics")

### Professional Work Section
- Overview: "What I build and why it matters"
- 5 projects (either inline or with links to `professional-work/` folder)
- Each project: problem → tech stack → architecture → outcomes → learnings

### Formal Education Section
- Brief summary: "3-year SNHU CS degree with focus on [domains]"
- Domain breakdown (Data Structures, Security, Graphics, etc.)
- Link to `courses/` for detailed course breakdown

### Footer
- Contact links (email, LinkedIn, GitHub)
- Last updated date

---

## Case Study Details

### Case Study Template

```markdown
## Project Name

**Problem:**
[1-2 sentences: What problem does it solve? Why does it matter to users/market?]

**Tech Stack:**
Backend: [languages/frameworks]
Frontend: [if applicable]
Database: [if applicable]
Deployment: [platform]
Hardware: [if applicable]

**Architecture:**
[2-3 sentences explaining key components + data flow]

[Optional: Mermaid diagram showing system flow]

**Key Challenges & Solutions:**

1. **Challenge:** [What was hard?]
   **Solution:** [How did you solve it?]

2. **Challenge:** [What was hard?]
   **Solution:** [How did you solve it?]

**Outcomes & Metrics:**
[What's the result? Users? Revenue? Performance? Learning outcomes?]

**Reflections:**
[What would you do differently? What did you learn?]
```

### Content Sources

For each project, information comes from:
- **GitHub repo metadata** (commits, branches, recent activity)
- **Memory files** (architecture decisions, phase completion, learnings)
- **Live deployments** (outcomes, metrics, where available—without sharing links)
- **Project README/docs** (where they exist in the repos)

---

## Architecture Diagrams

Each project includes a simple Mermaid diagram showing:
- **Data flow** (inputs → processing → outputs)
- **Key components** (backend, frontend, hardware, databases, external APIs)
- **System interactions** (how pieces talk to each other)

Diagrams are intentionally simple—visual proof of systems thinking, not exhaustive documentation.

**Example (Walter):**
```
Market Data APIs → Data Pipeline → Analytics Engine → Trading Logic → Broker API → Results
                                                                              ↓
                                                                        Dashboard Viewer
```

**Example (Curbly):**
```
Customer App ↔ Backend API ↔ MongoDB
               ↑
Hauler App ←→ Real-time GPS/Socket.io
```

---

## Coursework Repositioning

**Current state:** README emphasizes courses. Coursework dominates the portfolio.

**Target state:** Coursework becomes a secondary reference section.

**Implementation:**
1. Move coursework to "FORMAL EDUCATION" section (below professional work)
2. Provide brief domain summary (e.g., "Data Structures, Security, Graphics, Full-Stack Web")
3. Link to `courses/` folder for recruiters who want to verify formal education depth
4. Keep all existing course files unchanged

**Rationale:** Formal education validates technical breadth. Professional work proves shipping ability.

---

## Success Criteria

✓ Professional work is the headline (appears first in README)  
✓ Each project has clear problem statement + outcomes  
✓ Architecture diagrams show systems thinking  
✓ Hiring managers can understand what you built in <5 min per project  
✓ Coursework is discoverable but secondary  
✓ No deployment links revealed (security concern)  
✓ No private repo contents exposed  
✓ All documentation is in git and synced to public portfolio repo  

---

## Implementation Approach

**Phase 1: Gather & organize**
- Extract project summaries from GitHub repos
- Pull architecture/outcomes from memory files
- Create case study outlines

**Phase 2: Write case studies**
- 5 detailed case study markdown files in `professional-work/`
- Mermaid diagrams for each project
- Outcomes/metrics sourced from real data

**Phase 3: Restructure README**
- Rewrite main README.md with professional work first
- Link to case studies (or embed if short enough)
- Reorganize education section as secondary

**Phase 4: Review & publish**
- Commit to git
- Verify public portfolio shows professional work prominently
- Test that hiring managers can easily navigate projects

---

## Open Questions

None—design is complete and approved.

---

**Next Step:** Invoke writing-plans skill to create detailed implementation plan.
