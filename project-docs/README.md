# Project Documentation Series — xiao-mag-lora

This directory is the project's formal documentation set: a numbered
lifecycle series that runs from a structured requirements document, through
periodic progress reports, to a comprehensive completion report. It follows
the ISO/IEC/IEEE 12207 lifecycle-documentation shape (requirements
definition → project monitoring → closeout) with a NASA-style numbered
report scheme, kept lightweight for a small project.

## Document numbering

| ID    | Title                              | Path                      |
|-------|------------------------------------|---------------------------|
| 00    | Requirements (structured, REQ-IDs) | `00-requirements.md`      |
| 01-NN | Progress reports (chronological)   | `01-progress/NNN-<date>-status.md` |
| 02    | Completion report                  | `02-completion-report.md` |

Rules:

- **REQ-IDs are stable.** `00-requirements.md` assigns `REQ-001…REQ-0NN`.
  Progress reports and the completion report reference IDs, not prose.
  Requirements changes get a revision line in §6 of that document and are
  flagged in the next progress report.
- **Progress reports are append-only history.** Each report is
  `NNN-YYYY-MM-DD-status.md`, numbered sequentially, frozen once written.
  They state: work done since the previous report, REQ status changes,
  open items/risks, and next steps. They are short — status, not narrative.
- **The completion report is written last** (or as a living draft, marked
  `DRAFT`, in the final phase). It closes with the requirements
  traceability matrix — every REQ with a final status and evidence.
- **Evidence beats adjectives.** A REQ is "verified" only with a pointer to
  a test run, commit, bench log, or spec document.

## Relation to the rest of the repo

- `requirements/` — original client requirements (source of truth for intent;
  not revised).
- `docs/` — technical design documents (BOM, payload format, calculations,
  hardware strategy, PoC walkthrough). `00-requirements.md` links into these
  rather than duplicating them.
- This series is the *management* view: what is required, what is done, what
  remains, and how each requirement is satisfied.
