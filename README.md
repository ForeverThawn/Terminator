# Terminator

It's a temporary C++ project remastered from C (using struct instead of class), which means I would probably not maintain the code from time to time.

Just use it for basic usage. (when some program is stuck down, simply get a reset or termination)

Copyright (C) Forever 2020-2023

Usage

  -e / --execute       Execute a kernel command below
      "off"      --  Shutdown the computer immediately
      "reset"    --  Restart the computer immediately
      "error"    --  Throw blue screen error
      "hiber"    --  Hibernate the computer immediately
      "sleep"    --  Make the computer sleeping mode immediately

  -t / --timer         Specify a timer before anything is executed
      <#####>      --  Time as seconds

  --process-list       Show running process names and PIDs
       --sort-by       Sort the list by: (without this para means to sort by name)
       "name"        Image name (Default)
       "memory"      Process memory usage
       "pid"         Process ID

  --process-info       Show specified process info
      <#####>    --  Process PID

  --process-pid        Show specified process PID(s)
      <#####>    --  Process name

  --terminate-pid      Terminate the process by its PID
  --suspend-pid        Suspend the process by its PID
  --resume-pid         Resume the process by its PID
  --restart-pid        Restart the program completely by its PID
      <#####>    --  Process PID

  --terminate          Terminate the process by its name
  --suspend            Suspend the process by its name
  --resume             Resume the process by its name
      <#####>    --  Process name

  --memory-info        Look up memory
      "all"          --    Show all memory information
      "total"        --    Return the total physical memory size
      "avail"        --    Return the available physical memory size
      "page"         --    Return the page memory information
      "page_size"    --    Return the page memory size
      "page_total"   --    Return the total page num that the system can submit
      "page_limit"   --    Return the maximum page num that the system can submit
      "page_peak"    --    Return the maximum page num that the system already submitted
      "cache"        --    Return the cache memory size the system use by pages
      "kernel"       --    Return the kernel memory information
      "kernel_total" --    Return the total kernel memory size
      "kernel_paged" --    Return the paged kernel memory size
      "kernel_npaged"--    Return the non-paged kernel memory size
      "handle_count" --    Return the handle counts
      "process_count"--    Return the process counts
      "thread_count" --    Return the thread counts

  -h / --help          Show this help page
  -v / --version       Show the program version