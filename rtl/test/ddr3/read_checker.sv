`default_nettype none

module read_checker(
    input reset_n,
    input clk,

    input avl_rdata_valid,
    input [63:0] avl_rdata,

    input ddr3_init_done,
    input ddr3_cal_success,
    input ddr3_cal_fail,

    output logic is_finished,
    output logic pass,
    output logic fail);

    logic is_finished_next;
    logic pass_next;
    logic fail_next;

    localparam STATE_WAIT_FOR_INIT = 2'h0;
    localparam STATE_ERROR = 2'h1;
    localparam STATE_CHECK = 2'h2;
    localparam STATE_FINISHED = 2'h3;
    logic [1:0] state;
    logic [1:0] state_next;

    logic [24:0] test_counter;
    logic [24:0] test_counter_next;

    always_comb begin
        is_finished_next = is_finished;
        pass_next = pass;
        fail_next = fail;

        state_next = state;

        test_counter_next = test_counter;

        case (state)
            STATE_WAIT_FOR_INIT: begin
                if (ddr3_init_done) begin
                    if (ddr3_cal_success) begin
                        state_next = STATE_CHECK;
                    end
                    else if (ddr3_cal_fail) begin
                        state_next = STATE_ERROR;
                    end
                end
            end

            STATE_ERROR: begin
                is_finished_next = 1;
                fail_next = 1;
            end

            STATE_CHECK: begin
                if (test_counter[24]) begin
                    state_next = STATE_FINISHED;
                end
                else if (avl_rdata_valid) begin
                    if (avl_rdata == (64'hdeadfadebabebeef ^ {39'h0, test_counter})) begin
                        test_counter_next = test_counter + 25'h1;
                    end
                    else begin
                        state_next = STATE_ERROR;
                    end
                end
            end

            STATE_FINISHED: begin
                is_finished_next = 1;
                pass_next = 1;
            end
        endcase
    end

    always_ff @(posedge clk) begin
        if (!reset_n) begin
            is_finished <= 0;
            pass <= 0;
            fail <= 0;

            state <= STATE_WAIT_FOR_INIT;

            test_counter <= 25'h0;
        end
        else begin
            is_finished <= is_finished_next;
            pass <= pass_next;
            fail <= fail_next;

            state <= state_next;

            test_counter <= test_counter_next;
        end
    end

endmodule
