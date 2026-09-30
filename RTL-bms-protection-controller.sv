// ============================================================
// Adaptive Multi-Cell Battery Intelligence Engine
// RTL Battery Protection Controller
//
// States:
// 00 = NORMAL
// 01 = DEGRADED
// 10 = FAILSAFE
// 11 = SHUTDOWN
// ============================================================

module bms_protection_controller (
    input  logic clk,
    input  logic reset,

    input  logic minor_fault,
    input  logic critical_fault,
    input  logic pack_failure,

    output logic [1:0] state,
    output logic protection_active
);

    // State encoding
    localparam NORMAL   = 2'b00;
    localparam DEGRADED = 2'b01;
    localparam FAILSAFE = 2'b10;
    localparam SHUTDOWN = 2'b11;

    logic [1:0] next_state;

    // --------------------------------------------------------
    // Next-state logic
    // Highest-priority condition is PACK FAILURE
    // --------------------------------------------------------

    always_comb begin

        if (pack_failure)
            next_state = SHUTDOWN;

        else if (critical_fault)
            next_state = FAILSAFE;

        else if (minor_fault)
            next_state = DEGRADED;

        else
            next_state = NORMAL;

    end

    // --------------------------------------------------------
    // State register
    // --------------------------------------------------------

    always_ff @(posedge clk or posedge reset) begin

        if (reset)
            state <= NORMAL;

        else
            state <= next_state;

    end

    // --------------------------------------------------------
    // Protection output
    // Protection is active in FAILSAFE and SHUTDOWN
    // --------------------------------------------------------

    always_comb begin

        if ((state == FAILSAFE) || (state == SHUTDOWN))
            protection_active = 1'b1;

        else
            protection_active = 1'b0;

    end

endmodule
