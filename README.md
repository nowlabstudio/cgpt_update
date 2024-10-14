# GitHub Auto Uploader for Raspberry Pi 5 and AXIS Device

This project enables automatic monitoring of a GitHub repository from a Raspberry Pi 5 and uploads the latest code to an AXIS device (W6100 + RP2040). Below are the step-by-step instructions to set up the system.

## Requirements

1. **Raspberry Pi 5** (configured in headless mode).
2. **AXIS device** with RP2040 and W6100 Ethernet chip.
3. **GitHub Personal Access Token (PAT)** for accessing private repositories.
4. **Python 3** installed on the Raspberry Pi.
5. **Git** installed on the Raspberry Pi.
6. **Internet Access** for the Raspberry Pi and AXIS.

## Step-by-Step Instructions

### 1. Set Up the Raspberry Pi 5 in Headless Mode

- **Flash the SD Card**: Use the Raspberry Pi Imager to flash Raspberry Pi OS to your SD card.
  - Choose the Raspberry Pi OS (Recommended).
  - Configure settings (by clicking the gear icon) to enable SSH, set up the Wi-Fi, and provide username/password.
- **Enable SSH Manually (Optional)**:
  - If not done through the Pi Imager, add an empty file named `ssh` to the `boot` partition of the SD card to enable SSH access.
- **Connect to the Raspberry Pi**:
  - Insert the SD card, power up the Raspberry Pi, and find its IP address using your router or a network scanner.
  - Connect via SSH: `ssh pi@<IP_ADDRESS>`.

### 2. Install Dependencies on Raspberry Pi

Run the following commands to install Python 3, Git, and additional libraries:

```sh
sudo apt-get update
sudo apt-get install -y python3 git
python3 -m venv /home/raspi5/venv
echo 'source /home/raspi5/venv/bin/activate' >> ~/.bashrc
source /home/raspi5/venv/bin/activate
pip install gitpython requests
```

### 3. Clone the GitHub Repository and Monitor for Updates

Create the Python script (`github_auto_uploader.py`) in the directory `/home/raspi5/` with the following code:

```python
import os
import time
import git

# Configuration
REPO_PATH = "/home/raspi5/arduino_repo"  # Path where the repository will be cloned
GITHUB_REPO_URL = "https://<YOUR_PERSONAL_ACCESS_TOKEN>@github.com/nowlabstudio/cgpt_update.git"  # GitHub repository URL with Personal Access Token
CHECK_INTERVAL = 60  # Interval to check for updates (in seconds)

# Clone or open the repository
def get_repo():
    if not os.path.exists(REPO_PATH):
        print("Cloning repository...")
        git.Repo.clone_from(GITHUB_REPO_URL, REPO_PATH)
    return git.Repo(REPO_PATH)

# Main function to monitor the repository for updates
def main():
    repo = get_repo()
    origin = repo.remotes.origin

    while True:
        print("Checking for updates...")
        origin.fetch()
        if repo.head.commit != origin.refs.master.commit:
            print("New update found. Pulling changes...")
            origin.pull()
        else:
            print("No updates found.")

        time.sleep(CHECK_INTERVAL)

if __name__ == "__main__":
    main()
```

### 4. Configure GitHub Personal Access Token (PAT)

- Go to GitHub: **Settings > Developer Settings > Personal Access Tokens**.
- Generate a new token with permissions for accessing repositories (e.g., `repo` scope).
- Replace `<YOUR_PERSONAL_ACCESS_TOKEN>` in the script with your generated token.

### 5. Run the Script

Run the script on the Raspberry Pi:

```sh
python3 /home/raspi5/github_auto_uploader.py
```

The script will continuously monitor the specified GitHub repository for any updates and pull changes to the local folder (`/home/raspi5/arduino_repo`) every 60 seconds.

### 6. Preparing for Upload to the AXIS Device

In a future step, we will prepare the system to upload the downloaded code to the AXIS device automatically. The next iteration of the script will involve configuring a serial or Ethernet-based communication method to complete this task.

## Automate Script Launch on Boot

To ensure the script runs on boot, you can use `systemd`:

1. **Create a Service File**:

   ```sh
   sudo nano /etc/systemd/system/github_uploader.service
   ```

2. **Add the Following Content**:

   ```ini
   [Unit]
   Description=GitHub Auto Uploader Service
   After=network.target

   [Service]
   ExecStart=/usr/bin/python3 /home/raspi5/github_auto_uploader.py
   WorkingDirectory=/home/raspi5
   StandardOutput=inherit
   StandardError=inherit
   Restart=always
   User=pi

   [Install]
   WantedBy=multi-user.target
   ```

3. **Enable and Start the Service**:

   ```sh
   sudo systemctl enable github_uploader.service
   sudo systemctl start github_uploader.service
   ```

### 7. Testing and Debugging

- Use `journalctl -u github_uploader.service` to view logs and debug any issues.

## Next Steps

- **Step 2**: Prepare the Raspberry Pi to upload the downloaded code to the AXIS device via serial or Ethernet.
- **Step 3**: Finalize the upload script and test the complete workflow.

Feel free to reach out for any questions or assistance during the setup.

