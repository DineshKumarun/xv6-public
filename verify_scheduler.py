import sys
import pexpect
import time
import re

def run_benchmark(use_rl=False):
    print(f"[{'RL' if use_rl else 'Baseline'}] Starting QEMU...")
    # Restart QEMU fresh each time
    child = pexpect.spawn('make qemu-nox', encoding='utf-8')
    child.logfile = sys.stdout
    
    try:
        # Wait for initial shell
        child.expect(r'\$ ', timeout=30)
        
        if use_rl:
            print(f"[{'RL' if use_rl else 'Baseline'}] Starting RL Scheduler...")
            child.sendline('rl_scheduler &')
            # Wait for message
            child.expect(r'Starting Optimized RL Scheduler', timeout=10)
            # Sync prompt
            child.sendline('')
            child.expect(r'\$ ', timeout=10)
            time.sleep(2)

        print(f"[{'RL' if use_rl else 'Baseline'}] Running Benchmark...")
        child.sendline('time benchmark')
        
        # Expect prompt returning. Benchmark might be slow.
        # CPU ops are 200M. In QUX (Emulation), this might take a while.
        child.expect(r'\$ ', timeout=120)
        
        output = child.before
        
        # Look for "Real Time: X ticks"
        match = re.search(r'Real Time: (\d+) ticks', output, re.IGNORECASE)
        if match:
             ticks = int(match.group(1))
             print(f"\n[{'RL' if use_rl else 'Baseline'}] Result: {ticks} ticks")
             return ticks
        else:
             print(f"\n[{'RL' if use_rl else 'Baseline'}] Could not parse time.")
             # Debug: verify what we captured
             print("\n--- CAPTURED OUTPUT START ---")
             print(output.replace('\r', ''))
             print("--- CAPTURED OUTPUT END ---")
             return None

    except Exception as e:
        print(f"Error: {e}")
        return None
    finally:
        child.close()

def main():
    print("--- Running Verification ---")
    
    # Run Baseline
    baseline_ticks = run_benchmark(use_rl=False)
    # If baseline fails, we retry once? No, just stop.
    if baseline_ticks is None:
        print("Failed to get baseline. Aborting.")
        return

    time.sleep(1)

    # Run RL
    rl_ticks = run_benchmark(use_rl=True)
    if rl_ticks is None:
        print("Failed to get RL results. Aborting.")
        return

    print("\n--- Results ---")
    print(f"Baseline: {baseline_ticks} ticks")
    print(f"RL Agent: {rl_ticks} ticks")
    
    if rl_ticks < baseline_ticks:
        improvement = ((baseline_ticks - rl_ticks) / baseline_ticks) * 100
        print(f"SUCCESS: Performance improved by {improvement:.2f}%")
    else:
        print("FAIL: Performance did not improve (or was slower).")

if __name__ == "__main__":
    main()
