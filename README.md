# TTD-Bindings

Bindings (PoC) for [Microsoft WinDBG Time Travel Debugging (TTD)](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/time-travel-debugging-overview).

*This is for educational purposes only. Please consult and comply with the related Microsoft licensing*

## Usage

* `TTD/` is the main wrapper. `TTDReplay.dll` and `TTDReplayCPU.dll` (from WinDBG Preview, at least v1.2111) must be present in the same directory than the executable
* `example_api/` highlights some of the wrapping
* `example_diff/` shows how to use the wrapping to perform naive trace diffing

After performing one or several traces using Windbg Preview, one can open the `.run` file:
```C++
// Openning the trace
TTD::ReplayEngine ttdengine = TTD::ReplayEngine();
ttdengine.Initialize(L"D:\\traces\\demo.run");

// Retrieve information, such as the starting position
TTD::Position* first = ttdengine.GetFirstPosition();
printf("%llx:%llx\n", first->Major, first->Minor);

// Get a Cursor instance
TTD::Cursor ttdcursor = ttdengine.NewCursor();

// Set it to the first position
ttdcursor.SetPosition(first);

// Print the value of RCX
TTD::TTD_Replay_RegisterContext* ctxt = ttdcursor.GetCrossPlatformContext();
printf("RCX: %llx\n", ctxt->rcx);

// Make 3 step forward
TTD::TTD_Replay_ICursorView_ReplayResult replayresult;
TTD::Position* last = ttdengine.GetLastPosition();
ttdcursor.ReplayForward(&replayresult, last, 3);
```

Currently, the wrapping system supports:

* Openning a trace file
* Getting and setting the position
* Getting:
    * Peb address
    * Thread count
    * Module list
    * Program counter
    * Registers
* Query memory
* Replaying forward and backward
* Adding a callback for "CallRet"
* Memory breakpoints (R/W/X)

## Exemple: diffing path in a trace

Objective: find which instruction actually check for `PrimaryGroupID` possibilities when setting one in an Active Directory. For more information, please read [this article].(https://github.com/commial/experiments/tree/master/windows/random_things/primaryGroupID)

First, a trace is made on the Domain controller's `lsass.exe` while attempting to:

1. Set an un-authorized `PrimaryGroupID` to the user `simple`
1. Set an authorized `PrimaryGroupID` on the user `simple`

Then, the trace is loaded in Windbg Preview for analysis. We first find one of the entrypoint, for instance by looking at the callstack during some of the calls.
As a result, we saw that `ntdsai!LDAP_CONN::ModifyRequest` seems to be called when a LDAP request is made. This method is called twice, as expected:

![](timeline.png)

The first time, it quickly fails; the second, it processes the request, taking more time.

Now, here is the strategy:

1. Start from the first and the second call to `ntdsai!LDAP_CONN::ModifyRequest`
1. Step some instruction for both, and observe the program counter

* If it differ, get back to the last known sync point, and step less instruction
* If it is the same, continue from the new position

Using this method, we quickly end in `ntdll!RtlpAllocateHeap`. Indeed, while browsing the heap chunks, they might be some differences (as new chunk have been allocated between the two calls).

We know this is a false positive. To avoid it, we can:

1. Register a `CallReturnCallback` to track function calls and their return address
1. On detected diff, check if a function in the current call stack is in the allow-list and its corresponding return address is the same for both traces
1. If it is, forward both traces to the return address, and consider it a new sync point 

A stack of calls and return addresses is used, to handle cases such as:
```C
f(
    subcall()
    diff_call(
        // Difference is here, we want to go out of `f`
    )
)
```

This implementation is available in `example_diff`.
In addition to allocation function, a few others functions are allow-listed, such as column reading APIs.

As a result, the actual diff is quickly found:

```
Openning the trace
Openning the trace2
STEP 10000
Trace 1: 177b:3f        | Trace 2: 6871:3f
Trace 1 ends on 7ffb0b9cdca0    | Trace 2 ends on 7ffb0b9f2e2b
Rollback...OK
STEP 5000
Trace 1: 177b:3f        | Trace 2: 6871:3f
Trace 1 ends on 7ffb10ec1b5c    | Trace 2 ends on 7ffb10ec1c4d
Rollback...OK
STEP 2500
Trace 1: 177b:3f        | Trace 2: 6871:3f
Trace 1 ends on 7ffb10ec1d9b    | Trace 2 ends on 7ffb10ec1d9b
STEP 2500
Trace 1: 1788:21a       | Trace 2: 687e:21a
Trace 1 ends on 7ffb10ec1b5c    | Trace 2 ends on 7ffb10ec1c4d
Rollback...OK
STEP 1250
Trace 1: 1788:21a       | Trace 2: 687e:21a
Trace 1 ends on 7ffb10ebfc88    | Trace 2 ends on 7ffb10ec1b06
Rollback...OK
STEP 625
Trace 1: 1788:21a       | Trace 2: 687e:21a
Trace 1 ends on 7ffb0ba1f327    | Trace 2 ends on 7ffb0ba1eef4
Rollback...OK
STEP 312
Trace 1: 1788:21a       | Trace 2: 687e:21a
Trace 1 ends on 7ffb10f568d0    | Trace 2 ends on 7ffb10ec3ed3
Rollback...OK

...

STEP 2
Trace 1: 1788:229       | Trace 2: 687e:229
Trace 1 ends on 7ffb10ec1de8    | Trace 2 ends on 7ffb10ec1ec9
Rollback...OK
STEP 1
Trace 1: 1788:229       | Trace 2: 687e:229
Trace 1 ends on 7ffb10ec1de5    | Trace 2 ends on 7ffb10ec1ec1
Rollback...OK
Last known sync reference: Trace 1: 1788:229    | Trace 2: 687e:229
Cur func: 7ffb10ec1af0 -> 7ffb10ebfcad | 7ffb10ec1af0 -> 7ffb10ebfcad
Common func/ret:
7ffb0ba1efa0 -> 7ffb0b9aa1bc
        7ffb0bc0c8bc -> 7ffb0ba1f245
                7ffb0b9b7a9c -> 7ffb0bc0c8ec
                        7ffb0b9d37a0 -> 7ffb0b9b7ad7
                                7ffb10ebf2a0 -> 7ffb0b9d38bc
-> Resync traces after their RET on 7ffb0b9d38bc...TRACE1 7ffb0b9d38bc......TRACE2 7ffb0b9d38bc
STEP 10000

...

Rollback...OK
Last known sync reference: Trace 1: 1ad2:43     | Trace 2: 6b7e:43
Cur func: 7ffb0ca01f10 -> 7ffb0bac294d | 7ffb0ca01f10 -> 7ffb0bac294d
Common func/ret:
7ffb0b9f2848 -> 7ffb0b9aa396
        7ffb0b9f33b0 -> 7ffb0b9f29c6
                7ffb0b9f35ac -> 7ffb0b9f34eb
                        7ffb0b9909f4 -> 7ffb0b9f389e
                                7ffb0b991bc8 -> 7ffb0b990af0
                                        7ffb0b993334 -> 7ffb0b991f3e
                                                7ffb0ca01f10 -> 7ffb0bac294d
```

Going to `1ad2:43` and `6b7e:43`, we indeed ends on the expected check in `samsrv!SampAssignPrimaryGroup` \o/!

```
||0:0:001> !ttdext.tt 1ad2:43
Setting position: 1AD2:43
Time Travel Position: 1AD2:43
samsrv!SampAssignPrimaryGroup+0xb4:
00007ffb`0ca01fc4 7409            je      samsrv!SampAssignPrimaryGroup+0xbf (00007ffb`0ca01fcf) [br=0]
||0:0:001> k
 # Child-SP          RetAddr               Call Site
00 000000f0`006fd8b0 00007ffb`0bac294d     samsrv!SampAssignPrimaryGroup+0xb4
01 000000f0`006fd970 00007ffb`0b991f3e     ntdsai!SampDsSetInformationUser+0x12f619
02 000000f0`006fdb80 00007ffb`0b990af0     ntdsai!SampWriteSamAttributes+0x376
03 000000f0`006fdc90 00007ffb`0b9f389e     ntdsai!SampModifyLoopback+0xfc
04 000000f0`006fdd00 00007ffb`0b9f34eb     ntdsai!SampModifyLoopbackCheck+0x2f2
05 000000f0`006fddd0 00007ffb`0b9f29c6     ntdsai!LocalModify+0x13b
06 000000f0`006fe0f0 00007ffb`0b9aa396     ntdsai!DirModifyEntryNative+0x17e
07 000000f0`006fe450 00007ffb`0bd22ee9     ntdsai!LDAP_CONN::ModifyRequest+0x386
08 000000f0`006feac0 00007ffb`0b9c297a     ntdsai!LDAP_CONN::ModifyRequest+0x41
09 000000f0`006feb20 00007ffb`0ba1c6a2     ntdsai!LDAP_CONN::ProcessRequestEx+0xb6e
0a 000000f0`006fedf0 00007ffb`0ba1c069     ntdsai!LDAP_CONN::IoCompletion+0x51a
0b 000000f0`006ff650 00007ffb`0b912dd6     ntdsai!LdapCompletionRoutine+0x139
0c 000000f0`006ff6f0 00007ffb`0b912ae4     NTDSATQ!AtqpProcessContext+0xd6
0d 000000f0`006ff740 00007ffb`0fff7974     NTDSATQ!AtqPoolThread+0x1a4
0e 000000f0`006ff7d0 00007ffb`10f0a2f1     KERNEL32!BaseThreadInitThunk+0x14
0f 000000f0`006ff800 00000000`00000000     ntdll!RtlUserThreadStart+0x21
```

## References

* [MSDN - Time Travel Debugging - Overview](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/time-travel-debugging-overview)
* [TTD analysis](https://www.synacktiv.com/ressources/rumpinrennes-ttd.pdf)  by @w4kfu at Rump'in Rennes 2019
* [Initial iDNA paper](https://www.usenix.org/legacy/events/vee06/full_papers/p154-bhansali.pdf) : S. Bhansali, W.-K. Chen, S. de Jong, A. Edwards, R. Murray, M. Drinic, D. Mihocka, and J. Chau. Framework for "Instruction-level tracing and analysis of program executions" in VEE, 2006.