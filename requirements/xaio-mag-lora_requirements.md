Proposed Approach & Workflow

1.  Write Requirements
2.  Generate a Hardware Strategy
      - Documentation Reference: Seeed Studio Getting Started (https://wiki.seeedstudio.com/Getting_Started/)
      - Hardware Components:
          - Seeed Studio Wio-S3 Wireless LoRa Module  (x2) https://www.seeedstudio.com/Wio-S3-Wireless-Module-p-6832.html [PDF: https://files.seeedstudio.com/Bazaar/product_pdf/100020327.pdf]
          - 2.4GHz FPC Antenna A-02 for XIAO ESP32-S3 (x2) https://www.seeedstudio.com/2-4GHz-FPC-Antenna-1-16dBi-for-XIAO-ESP32S3-p-6440.html [PDF: https://files.seeedstudio.com/Bazaar/product_pdf/318020968.pdf]
          - L76K GNSS Module for Seeed Studio XIAO (x1) — PDF Datasheet (https://files.seeedstudio.com/Bazaar/product_pdf/109100021.pdf)
          - Solar Panel (Needs discussion regarding peak voltage & power management. Option: Adapt a SenseCAP Solar Node P1-Pro for Meshtastic)	
			- Suitable small footprint Solar panel to  LiFePo charge controller -- from Seed Studio if possible. [To manage four 18650 cells])
          - 2.4 GHz FPC Antenna A-02 for XIAO ESP32-S3 (for external configuration via Bluetooth/WiFi) (x2)
          - Housing: 2" PVC Schedule 40 Pipe with end caps & PVC Cement (x1)
          - Mounting Hardware: 1"–1.5" PVC Schedule 40 Pipe with end caps
      - Note: If ordering from Seeed Studio, ensure items are in stock at their US warehouse to avoid shipping delays.
(I will also be mailing key pieces of this kit for your evaluation in a few days)
3.  Create an Overall Bill of Materials (BOM)
4.  Set Up Project Repository & Environment
      - Create a project folder (e.g., xiao-mag-halo if it does not exist.).
      - Add subfolders: assets, docs, src, dist, and a KiCad project folder.
      - Add a software_requirements.md file to the main folder.
      - Initialize Git (git init) on the top-level directory and confirm GitHub access. (only if this has not already been done)
5.  Develop Hardware & Software
      - Create layout for supporting hardware in KiCad (e.g., similar to existing RM3100 remote designs).
      - Choose a software development toolkit (Arduino, PlatformIO, or ESP-IDF).
      - Build, install, and test software.
6.  Integration, Deployment, and Evaluation
      - Assemble hardware and test the combined system.
      - Deploy initial systems for evaluation.
      - Verify basic functionality and evaluate performance goals and limits.
7.	Client-side (Linux host) control software should be written in any combination of golang, C/C++, Rust needed for security and portability across popular Linux distrubutions.  We will be creating a SPA Web app for tests and monitoring. 

Please let me know how you would like to proceed or if you prefer a different direction for researching the project solution and hardware options.

