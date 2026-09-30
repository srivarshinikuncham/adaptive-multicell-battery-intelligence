// ============================================================
// Testbench for Adaptive Multi-Cell Battery Protection RTL
// ============================================================

`timescale 1ns/1ps

module testbench;

    logic clk;
    logic reset;

    logic minor_fault;
    logic critical_fault;
    logic pack_failure;

    logic [1:0] state;
    logic protection_active;

    // --------------------------------------------------------
    // Instantiate the RTL design
    // --------------------------------------------------------

    bms_protection_controller DUT (
        .clk(clk),
        .reset(reset),
        .minor_fault(minor_fault),
        .critical_fault(critical_fault),
        .pack_failure(pack_failure),
        .state(state),
        .protection_active(protection_active)
    );

    // --------------------------------------------------------
    // Clock generation
    // 10 ns clock period
    // --------------------------------------------------------

    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    // --------------------------------------------------------
    // Waveform recording
    // --------------------------------------------------------

    initial begin
        $dumpfile("bms_waveform.vcd");
        $dumpvars(0, testbench);
    end

    // --------------------------------------------------------
    // Display simulation results
    // --------------------------------------------------------

    initial begin
        $monitor(
            "TIME=%0t | RESET=%b | MINOR=%b | CRITICAL=%b | FAILURE=%b | STATE=%b | PROTECTION=%b",
            $time,
            reset,
            minor_fault,
            critical_fault,
            pack_failure,
            state,
            protection_active
        );
    end

    // --------------------------------------------------------
    // Test scenarios
    // --------------------------------------------------------

    initial begin

        // Initial values
        reset = 1;
        minor_fault = 0;
        critical_fault = 0;
        pack_failure = 0;

        #12;

        // ----------------------------------------------------
        // TEST 1: HEALTHY
        // Expected: NORMAL
        // ----------------------------------------------------

        reset = 0;

        #20;

        // ----------------------------------------------------
        // TEST 2: MINOR IMBALANCE
        // Expected: DEGRADED
        // ----------------------------------------------------

        minor_fault = 1;

        #20;

        // ----------------------------------------------------
        // TEST 3: CRITICAL CONDITION
        // Expected: FAILSAFE
        // ----------------------------------------------------

        minor_fault = 0;
        critical_fault = 1;

        #20;

        // ----------------------------------------------------
        // TEST 4: PACK FAILURE
        // Expected: SHUTDOWN
        // ----------------------------------------------------

        critical_fault = 0;
        pack_failure = 1;

        #20;

        // ----------------------------------------------------
        // TEST 5: RECOVERY
        // Expected: NORMAL
        // ----------------------------------------------------

        pack_failure = 0;

        #20;

        // ----------------------------------------------------
        // End simulation
        // ----------------------------------------------------

        $finish;

    end

endmodule
