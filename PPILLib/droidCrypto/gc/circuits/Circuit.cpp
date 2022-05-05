    #include <droidCrypto/gc/HalfGate.h>
    #include <droidCrypto/gc/WireLabel.h>
    #include <droidCrypto/gc/circuits/Circuit.h>
    #include <assert.h>
    #include <droidCrypto/utils/Log.h>
    #include <endian.h>
    #include <chrono>
    #include "droidCrypto/as/ArithmeticShareEnviroment.h"
    namespace droidCrypto {

    void Circuit::garble(const std::vector<BitVector> &database, int protNum, CommunicationHandler& comm) {
        Log::v("GC", "Start Garble");
        Garbler g(channel);
        int bytesSent1 = channel.getBytesSent();
        int bytesRecv1 = channel.getBytesRecv();
        auto time1 = std::chrono::high_resolution_clock::now();

        g.performBaseOTs();

        auto time2 = std::chrono::high_resolution_clock::now();
        int bytesSent2 = channel.getBytesSent();
        int bytesRecv2 = channel.getBytesRecv();
        bytesSentBaseOT = bytesSent2 - bytesSent1;
        bytesRecvBaseOT = bytesRecv2 - bytesRecv1;
        timeBaseOT = time2 - time1;
        auto BaseOTTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeBaseOT);
        comm.saveBenchmark(comm.BASE_OT_GC_TIME, BaseOTTime.count());
        comm.saveBenchmark(comm.BASE_OT_GC_SENT, bytesSentBaseOT);
        comm.saveBenchmark(comm.BASE_OT_GC_RECV, bytesRecvBaseOT);
        Log::v("GC", "BaseOT Done in %f seconds", timeBaseOT);
        Log::v("GC", "BaseOT communication: %d bytes sent, %d bytes received", bytesSentBaseOT, bytesRecvBaseOT);

        std::vector<std::vector<WireLabel>> aliceInputs;
        for(int i = 0; i < database.size(); i++){
            std::vector<WireLabel> aliceInput = g.inputOfAlice(database[i]);
            aliceInputs.push_back(aliceInput);
        }

        auto time2a = std::chrono::high_resolution_clock::now();
        int bytesSent2a = channel.getBytesSent();
        int bytesRecv2a = channel.getBytesRecv();
        int bytesSentCircuit = bytesSent2a - bytesSent2;
        int bytesRecvCircuit = bytesRecv2a - bytesRecv2;
        auto sendingCircuit = time2a - time2;
        auto CircuitSendingTime = std::chrono::duration_cast<std::chrono::milliseconds>(sendingCircuit);
        comm.saveBenchmark(comm.GC_SENDING_INPUTS_TIME, CircuitSendingTime.count());
        comm.saveBenchmark(comm.GC_SENDING_INPUTS_SENT, bytesSentCircuit);
        comm.saveBenchmark(comm.GC_SENDING_INPUTS_RECV, bytesRecvCircuit);
        Log::v("GC", "Sending of the inputs Done in %f seconds", sendingCircuit);
        Log::v("GC", "Sending of the inputs communication: %d bytes sent, %d bytes received", bytesSentCircuit, bytesRecvCircuit);

        std::vector<WireLabel> bobInput;
        std::vector<std::vector<WireLabel>> bobInputs;
        if(protNum == 0){
            bobInput = g.inputOfBob(mInputB_size);
        } else {
            bobInput = g.inputOfBob(mInputB_size * database.size());
            int h =0;
            while (h < bobInput.size()) {
                std::vector<WireLabel> in;
                for (int j = 0; j < mInputB_size; ++j) {
                    in.push_back(bobInput[h]);
                    h++;
                }
                bobInputs.push_back(in);
            }
        }

        auto time3 = std::chrono::high_resolution_clock::now();
        int bytesSent3 = channel.getBytesSent();
        int bytesRecv3 = channel.getBytesRecv();
        bytesSentOT = bytesSent3 - bytesSent2a;
        bytesRecvOT = bytesRecv3 - bytesRecv2a;
        timeOT = time3 - time2a;
        auto OTTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeOT);
        comm.saveBenchmark(comm.GC_OT_TIME, OTTime.count());
        comm.saveBenchmark(comm.GC_OT_SENT, bytesSentOT);
        comm.saveBenchmark(comm.GC_OT_RECV, bytesRecvOT);
        Log::v("GC", "OT done in %f seconds", timeOT);
        Log::v("GC", "OT communication: %d bytes sent, %d bytes received", bytesSentOT, bytesRecvOT);
        std::vector<std::vector<WireLabel>> outputs;
        if (protNum == 0){
            outputs = computeFunction(aliceInputs, bobInput, g);
        } else{
            outputs = computeFunction(aliceInputs, bobInputs, g);
        }

        auto time4 = std::chrono::high_resolution_clock::now();
        int bytesSent4 = channel.getBytesSent();
        int bytesRecv4 = channel.getBytesRecv();
        timeEval = time4 - time3;
        bytesSentEval = bytesSent4 - bytesSent3;
        bytesRecvEval = bytesRecv4 - bytesRecv3;
        timeNetworkCircuit = timeEval - g.timeGarbling;
        comm.saveBenchmark(comm.GC_EVALUATION_TIME, 0);
        auto garbleTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeEval);
        comm.saveBenchmark(comm.GC_GARBLING_TIME, garbleTime.count());
        comm.saveBenchmark(comm.GC_GARBLING_SENT, bytesSentEval);
        comm.saveBenchmark(comm.GC_GARBLING_RECV, bytesRecvEval);
        comm.saveBenchmark(comm.GC_NUM_XOR, g.getNumXORs());
        comm.saveBenchmark(comm.GC_NUM_AND, g.getNumANDs());
        Log::v("GC", "Garbling done in %f seconds", g.timeGarbling);
        Log::v("GC", "Garbling communication: %d bytes sent, %d bytes received", bytesSentEval, bytesRecvEval);
        Log::v("GC", "numANDs: %zu", g.getNumANDs());
        Log::v("GC", "numXORs: %zu", g.getNumXORs());

        for(int i = 0; i < outputs.size(); ++i) {
            g.outputToBob(outputs[i]);
        }

        auto time5 = std::chrono::high_resolution_clock::now();
        int bytesSent5 = channel.getBytesSent();
        int bytesRecv5 = channel.getBytesRecv();
        bytesSentOutput = bytesSent5 - bytesSent4;
        bytesRecvOutput = bytesRecv5 - bytesRecv4;
        timeOutput = time5 - time4;
        auto outputTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeOutput);
        comm.saveBenchmark(comm.GC_OUTPUT_TIME, outputTime.count());
        comm.saveBenchmark(comm.GC_OUTPUT_SENT, bytesSentOutput);
        comm.saveBenchmark(comm.GC_OUTPUT_RECV, bytesRecvOutput);
        Log::v("GC", "Outputs send to Client in %f seconds", timeOutput);
        Log::v("GC", "Output communication: %d bytes sent, %d bytes received", bytesSentOutput, bytesRecvOutput);
        timeTotal = time5 -time1;
        bytesSentTotal = bytesSent5 - bytesSent1;
        bytesRecvTotal = bytesRecv5 - bytesRecv1;
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeTotal);
        comm.saveBenchmark(comm.GC_TOTAL_TIME, totalTime.count());
        comm.saveBenchmark(comm.GC_TOTAL_SENT, bytesSentTotal);
        comm.saveBenchmark(comm.GC_TOTAL_RECV, bytesRecvTotal);
        Log::v("GC", "Total GC time was %f seconds", timeTotal);
        Log::v("GC", "Total communication: %d bytes sent, %d bytes received", bytesSentTotal, bytesRecvTotal);
        channel.clearStats();
        return;
    }

    std::vector<BitVector> Circuit::evaluate(const BitVector &fingerprint, int m, CommunicationHandler& comm) {
        Evaluator e(channel);
        auto time1 = std::chrono::high_resolution_clock::now();
        int bytesSent1 = channel.getBytesSent();
        int bytesRecv1 = channel.getBytesRecv();
        e.performBaseOTs();

        auto time2 = std::chrono::high_resolution_clock::now();
        timeBaseOT = time2 - time1;
        Log::v("GC", "BaseOT Done in %f seconds", timeBaseOT);
        int bytesSent2 = channel.getBytesSent();
        int bytesRecv2 = channel.getBytesRecv();
        bytesSentBaseOT = bytesSent2 - bytesSent1;
        bytesRecvBaseOT = bytesRecv2 - bytesRecv1;
        Log::v("GC", "BaseOT communication: %d bytes sent, %d bytes received", bytesSentBaseOT, bytesRecvBaseOT);
        auto BaseOTTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeBaseOT);
        comm.saveBenchmark(comm.BASE_OT_GC_TIME, BaseOTTime.count());
        comm.saveBenchmark(comm.BASE_OT_GC_SENT, bytesSentBaseOT);
        comm.saveBenchmark(comm.BASE_OT_GC_RECV, bytesRecvBaseOT);


        assert(fingerprint.size() == mInputB_size);
        std::vector<std::vector<WireLabel>> aliceInputs;
        for (int i = 0; i < m; ++i) {
            std::vector<WireLabel> aliceInput = e.inputOfAlice(mInputA_size);
            aliceInputs.push_back(aliceInput);
        }

        auto time2a = std::chrono::high_resolution_clock::now();
        int bytesSent2a = channel.getBytesSent();
        int bytesRecv2a = channel.getBytesRecv();
        int bytesSentCircuit = bytesSent2a - bytesSent2;
        int bytesRecvCircuit = bytesRecv2a - bytesRecv2;
        auto sendingCircuit = time2a - time2;
        auto CircuitSendingTime = std::chrono::duration_cast<std::chrono::milliseconds>(sendingCircuit);
        comm.saveBenchmark(comm.GC_SENDING_INPUTS_TIME, CircuitSendingTime.count());
        comm.saveBenchmark(comm.GC_SENDING_INPUTS_SENT, bytesSentCircuit);
        comm.saveBenchmark(comm.GC_SENDING_INPUTS_RECV, bytesRecvCircuit);
        Log::v("GC", "Sending of the inputs Done in %f seconds", sendingCircuit);
        Log::v("GC", "Sending of the inputs communication: %d bytes sent, %d bytes received", bytesSentCircuit, bytesRecvCircuit);

        std::vector<WireLabel> bobInput = e.inputOfBob(fingerprint);

        auto time3 = std::chrono::high_resolution_clock::now();
        int bytesSent3 = channel.getBytesSent();
        int bytesRecv3 = channel.getBytesRecv();
        bytesSentOT = bytesSent3 - bytesSent2a;
        bytesRecvOT = bytesRecv3 - bytesRecv2a;
        timeOT = time3 - time2a;
        auto OTTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeOT);
        comm.saveBenchmark(comm.GC_OT_TIME, OTTime.count());
        comm.saveBenchmark(comm.GC_OT_SENT, bytesSentOT);
        comm.saveBenchmark(comm.GC_OT_RECV, bytesRecvOT);
        Log::v("GC", "OT done in %f seconds", timeOT);
        Log::v("GC", "OT communication: %d bytes sent, %d bytes received", bytesSentOT, bytesRecvOT);

        std::vector<std::vector<WireLabel>> outputs = computeFunction(aliceInputs, bobInput, e);

        auto time4 = std::chrono::high_resolution_clock::now();
        timeEval = time4 - time3;
        timeNetworkCircuit = timeEval - e.timeEvaluation;
        Log::v("GC","Evaluation done in %f seconds",e.timeEvaluation);
        Log::v("GC", "Networking time: %f", timeNetworkCircuit);
        int bytesSent4 = channel.getBytesSent();
        int bytesRecv4 = channel.getBytesRecv();
        bytesSentEval = bytesSent4 - bytesSent3;
        bytesRecvEval = bytesRecv4 - bytesRecv3;
        auto EvalTime = std::chrono::duration_cast<std::chrono::milliseconds>(e.timeEvaluation);
        comm.saveBenchmark(comm.GC_EVALUATION_TIME, EvalTime.count());
        comm.saveBenchmark(comm.GC_GARBLING_SENT, bytesSentEval);
        comm.saveBenchmark(comm.GC_GARBLING_RECV, bytesRecvEval);
        comm.saveBenchmark(comm.GC_NUM_XOR, e.getNumXORs());
        comm.saveBenchmark(comm.GC_NUM_AND, e.getNumANDs());
        Log::v("GC", "Evaluation communication: %d bytes sent, %d bytes received", bytesSentEval, bytesRecvEval);
        Log::v("GC", "numANDs: %zu", e.getNumANDs());
        Log::v("GC", "numXORs: %zu", e.getNumXORs());

        std::vector<BitVector> result;
        for (int i = 0; i < outputs.size(); ++i) {
            BitVector output = e.outputToBob(outputs[i]);
            result.push_back(output);
        }

        auto time5 = std::chrono::high_resolution_clock::now();
        timeOutput = time5 - time4;
        Log::v("GC","Output received after %f seconds",timeOutput);
        int bytesSent5 = channel.getBytesSent();
        int bytesRecv5 = channel.getBytesRecv();
        bytesSentOutput = bytesSent5 - bytesSent4;
        bytesRecvOutput = bytesRecv5 - bytesRecv4;
        auto outputTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeOutput);
        comm.saveBenchmark(comm.GC_OUTPUT_TIME, outputTime.count());
        comm.saveBenchmark(comm.GC_OUTPUT_SENT, bytesSentOutput);
        comm.saveBenchmark(comm.GC_OUTPUT_RECV, bytesRecvOutput);
        Log::v("GC", "Output communication: %d bytes sent, %d bytes received", bytesSentOutput, bytesRecvOutput);

        timeTotal = time5 -time1;
        Log::v("GC", "Total GC time was %f seconds", timeTotal);
        bytesSentTotal = bytesSent5 - bytesSent1;
        bytesRecvTotal = bytesRecv5 - bytesRecv1;
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeTotal);
        comm.saveBenchmark(comm.GC_TOTAL_TIME, totalTime.count());
        comm.saveBenchmark(comm.GC_TOTAL_SENT, bytesSentTotal);
        comm.saveBenchmark(comm.GC_TOTAL_RECV, bytesSentTotal);
        Log::v("GC", "Total communication: %d bytes sent, %d bytes received", bytesSentTotal, bytesRecvTotal);

        return result;
        }

        std::vector<BitVector> Circuit::evaluate(const std::vector<BitVector> &distanceShares, CommunicationHandler& comm) {
        Log::v("GC", "Start Eval");
            Evaluator e(channel);
            int bytesSent1 = channel.getBytesSent();
            int bytesRecv1 = channel.getBytesRecv();
            auto time1 = std::chrono::high_resolution_clock::now();

            e.performBaseOTs();

            auto time2 = std::chrono::high_resolution_clock::now();
            timeBaseOT = time2 - time1;
            Log::v("GC", "BaseOT Done in %f seconds", timeBaseOT);
            int bytesSent2 = channel.getBytesSent();
            int bytesRecv2 = channel.getBytesRecv();
            bytesSentBaseOT = bytesSent2 - bytesSent1;
            bytesRecvBaseOT = bytesRecv2 - bytesRecv1;
            Log::v("GC", "BaseOT communication: %d bytes sent, %d bytes received", bytesSentBaseOT, bytesRecvBaseOT);
            auto BaseOTTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeBaseOT);
            comm.saveBenchmark(comm.BASE_OT_GC_TIME, BaseOTTime.count());
            comm.saveBenchmark(comm.BASE_OT_GC_SENT, bytesSentBaseOT);
            comm.saveBenchmark(comm.BASE_OT_GC_RECV, bytesRecvBaseOT);

            std::vector<std::vector<WireLabel>> aliceInputs;
            std::vector<std::vector<WireLabel>> bobInputs;
            for (int i = 0; i < distanceShares.size(); ++i) {
                std::vector<WireLabel> aliceInput = e.inputOfAlice(mInputA_size);
                aliceInputs.push_back(aliceInput);
            }

            auto time2a = std::chrono::high_resolution_clock::now();
            int bytesSent2a = channel.getBytesSent();
            int bytesRecv2a = channel.getBytesRecv();
            int bytesSentCircuit = bytesSent2a - bytesSent2;
            int bytesRecvCircuit = bytesRecv2a - bytesRecv2;
            auto sendingCircuit = time2a - time2;
            auto CircuitSendingTime = std::chrono::duration_cast<std::chrono::milliseconds>(sendingCircuit);
            comm.saveBenchmark(comm.GC_SENDING_INPUTS_TIME, CircuitSendingTime.count());
            comm.saveBenchmark(comm.GC_SENDING_INPUTS_SENT, bytesSentCircuit);
            comm.saveBenchmark(comm.GC_SENDING_INPUTS_RECV, bytesRecvCircuit);
            Log::v("GC", "Sending of the inputs Done in %f seconds", sendingCircuit);
            Log::v("GC", "Sending of the inputs communication: %d bytes sent, %d bytes received", bytesSentCircuit, bytesRecvCircuit);

            BitVector bobInput;
            for (int i = 0; i < distanceShares.size(); ++i) {
                bobInput.append(distanceShares[i]);
            }
            std::vector<WireLabel> inputs = e.inputOfBob(bobInput);
            int h =0;
            while (h < inputs.size()) {
                std::vector<WireLabel> in;
                for (int j = 0; j < mInputB_size; ++j) {
                    in.push_back(inputs[h]);
                    h++;
                }
                bobInputs.push_back(in);
            }

            auto time3 = std::chrono::high_resolution_clock::now();
            int bytesSent3 = channel.getBytesSent();
            int bytesRecv3 = channel.getBytesRecv();
            bytesSentOT = bytesSent3 - bytesSent2a;
            bytesRecvOT = bytesRecv3 - bytesRecv2a;
            timeOT = time3 - time2a;
            auto OTTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeOT);
            comm.saveBenchmark(comm.GC_OT_TIME, OTTime.count());
            comm.saveBenchmark(comm.GC_OT_SENT, bytesSentOT);
            comm.saveBenchmark(comm.GC_OT_RECV, bytesRecvOT);
            Log::v("GC", "OT done in %f seconds", timeOT);
            Log::v("GC", "OT communication: %d bytes sent, %d bytes received", bytesSentOT, bytesRecvOT);


            std::vector<std::vector<WireLabel>> outputs = computeFunction(aliceInputs, bobInputs, e);

            auto time4 = std::chrono::high_resolution_clock::now();
            timeEval = time4 - time3;
            timeNetworkCircuit = timeEval - e.timeEvaluation;
            Log::v("GC","Evaluation done in %f seconds",e.timeEvaluation);
            Log::v("GC", "Networking time: %f", timeNetworkCircuit);
            int bytesSent4 = channel.getBytesSent();
            int bytesRecv4 = channel.getBytesRecv();
            bytesSentEval = bytesSent4 - bytesSent3;
            bytesRecvEval = bytesRecv4 - bytesRecv3;
            auto EvalTime = std::chrono::duration_cast<std::chrono::milliseconds>(e.timeEvaluation);
            comm.saveBenchmark(comm.GC_EVALUATION_TIME, EvalTime.count());
            comm.saveBenchmark(comm.GC_GARBLING_SENT, bytesSentEval);
            comm.saveBenchmark(comm.GC_GARBLING_RECV, bytesRecvEval);
            comm.saveBenchmark(comm.GC_NUM_XOR, e.getNumXORs());
            comm.saveBenchmark(comm.GC_NUM_AND, e.getNumANDs());
            Log::v("GC", "Evaluation communication: %d bytes sent, %d bytes received", bytesSentEval, bytesRecvEval);
            Log::v("GC", "numANDs: %zu", e.getNumANDs());
            Log::v("GC", "numXORs: %zu", e.getNumXORs());

            std::vector<BitVector> result;
            for (int i = 0; i < outputs.size(); ++i) {
                BitVector output = e.outputToBob(outputs[i]);
                result.push_back(output);
            }

            auto time5 = std::chrono::high_resolution_clock::now();
            timeOutput = time5 - time4;
            Log::v("GC","Output received after %f seconds",timeOutput);
            int bytesSent5 = channel.getBytesSent();
            int bytesRecv5 = channel.getBytesRecv();
            bytesSentOutput = bytesSent5 - bytesSent4;
            bytesRecvOutput = bytesRecv5 - bytesRecv4;
            auto outputTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeOutput);
            comm.saveBenchmark(comm.GC_OUTPUT_TIME, outputTime.count());
            comm.saveBenchmark(comm.GC_OUTPUT_SENT, bytesSentOutput);
            comm.saveBenchmark(comm.GC_OUTPUT_RECV, bytesRecvOutput);
            Log::v("GC", "Output communication: %d bytes sent, %d bytes received", bytesSentOutput, bytesRecvOutput);

            timeTotal = time5 -time1;
            Log::v("GC", "Total GC time was %f seconds", timeTotal);
            bytesSentTotal = bytesSent5 - bytesSent1;
            bytesRecvTotal = bytesRecv5 - bytesRecv1;
            Log::v("GC", "Total communication: %d bytes sent, %d bytes received", bytesSentTotal, bytesRecvTotal);
            auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeTotal);
            comm.saveBenchmark(comm.GC_TOTAL_TIME, totalTime.count());
            comm.saveBenchmark(comm.GC_TOTAL_SENT, bytesSentTotal);
            comm.saveBenchmark(comm.GC_TOTAL_RECV, bytesRecvTotal);

            return result;
        }
//Not used -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        void Circuit::garble(const BitVector &inputA) {
            Garbler g(channel);
            auto time1 = std::chrono::high_resolution_clock::now();

            g.performBaseOTs();

            auto time2 = std::chrono::high_resolution_clock::now();
            timeBaseOT = time2 - time1;

            assert(inputA.size() == mInputA_size);
            std::vector<WireLabel> aliceInput = g.inputOfAlice(inputA);
            std::vector<WireLabel> bobInput = g.inputOfBob(mInputB_size);

            auto time3 = std::chrono::high_resolution_clock::now();
            timeOT = time3 - time2;

            std::vector<WireLabel> outputs = computeFunction(aliceInput, bobInput, g);

            auto time4 = std::chrono::high_resolution_clock::now();
            timeEval = time4 - time3;

            g.outputToBob(outputs);
            auto time5 = std::chrono::high_resolution_clock::now();
            timeOutput = time5 - time4;

            return;
        }

        BitVector Circuit::evaluate(const BitVector &inputB) {
            Evaluator e(channel);
            auto time1 = std::chrono::high_resolution_clock::now();

            e.performBaseOTs();

            auto time2 = std::chrono::high_resolution_clock::now();
            timeBaseOT = time2 - time1;

            assert(inputB.size() == mInputB_size);
            std::vector<WireLabel> aliceInput = e.inputOfAlice(mInputA_size);
            std::vector<WireLabel> bobInput = e.inputOfBob(inputB);

            auto time3 = std::chrono::high_resolution_clock::now();
            timeOT = time3 - time2;

            std::vector<WireLabel> outputs = computeFunction(aliceInput, bobInput, e);

            auto time4 = std::chrono::high_resolution_clock::now();
            timeEval = time4 - time3;

            Log::v("GC", "numANDs: %zu", e.getNumANDs());
            Log::v("GC", "numXORs: %zu", e.getNumXORs());

            BitVector output = e.outputToBob(outputs);
            auto time5 = std::chrono::high_resolution_clock::now();
            timeOutput = time5 - time4;

            return output;
        }
    //------------------------------------------------------------------------------------------------------------------
    // SIMD

    void SIMDCircuit::garble(const BitVector &inputA, const size_t SIMDvalues) {
      size_t transfer = htobe64(SIMDvalues);
      channel.send((uint8_t *)&transfer, sizeof(transfer));
      SIMDGarbler g(channel, SIMDvalues);
      auto time1 = std::chrono::high_resolution_clock::now();

      g.performBaseOTs();

      auto time2 = std::chrono::high_resolution_clock::now();
      timeBaseOT = time2 - time1;

      assert(inputA.size() == mInputA_size);
      std::vector<WireLabel> aliceInput = g.inputOfAlice(inputA);
      std::vector<SIMDWireLabel> bobInput = g.inputOfBob(mInputB_size);

      auto time3 = std::chrono::high_resolution_clock::now();
      timeOT = time3 - time2;

      std::vector<SIMDWireLabel> outputs = computeFunction(aliceInput, bobInput, g);

      auto time4 = std::chrono::high_resolution_clock::now();
      timeEval = time4 - time3;

      g.outputToBob(outputs);
      auto time5 = std::chrono::high_resolution_clock::now();
      timeOutput = time5 - time4;

      return;
    }

    void SIMDCircuit::garbleSIMD(const std::vector<BitVector> &inputA) {
      const size_t SIMDvalues = inputA.size();
      size_t transfer = htobe64(SIMDvalues);
      channel.send((uint8_t *)&transfer, sizeof(transfer));
      SIMDGarbler g(channel, SIMDvalues);
      auto time1 = std::chrono::high_resolution_clock::now();

      g.performBaseOTs();

      auto time2 = std::chrono::high_resolution_clock::now();
      timeBaseOT = time2 - time1;

      assert(inputA.front().size() == mInputA_size);
      std::vector<SIMDWireLabel> aliceInput = g.inputOfAlice(inputA);
      std::vector<SIMDWireLabel> bobInput = g.inputOfBob(mInputB_size);

      auto time3 = std::chrono::high_resolution_clock::now();
      timeOT = time3 - time2;

      std::vector<SIMDWireLabel> outputs = computeFunction(aliceInput, bobInput, g);

      auto time4 = std::chrono::high_resolution_clock::now();
      timeEval = time4 - time3;

      g.outputToBob(outputs);
      auto time5 = std::chrono::high_resolution_clock::now();
      timeOutput = time5 - time4;

      return;
    }

    std::vector<BitVector> SIMDCircuit::evaluate(
        const std::vector<BitVector> &inputB) {
      size_t transfer;
      channel.recv((uint8_t *)&transfer, sizeof(transfer));
      transfer = be64toh(transfer);
      const size_t SIMDvalues = inputB.size();
      Log::v("GC", "SIMD: %zu, transfer:%zu", SIMDvalues, transfer);
      assert(SIMDvalues == transfer);

      SIMDEvaluator e(channel, SIMDvalues);
      auto time1 = std::chrono::high_resolution_clock::now();

      e.performBaseOTs();
      Log::v("GC", "baseOTs done");

      auto time2 = std::chrono::high_resolution_clock::now();
      timeBaseOT = time2 - time1;

      assert(inputB.front().size() == mInputB_size);
      std::vector<WireLabel> aliceInput = e.inputOfAlice(mInputA_size);
      Log::v("GC", "inputA done");

      Log::v("GC", "inputA sent: %zu, recv: %zu", channel.getBytesSent(),
             channel.getBytesRecv());
      std::vector<SIMDWireLabel> bobInput = e.inputOfBob(inputB);

      Log::v("GC", "inputB done");

      auto time3 = std::chrono::high_resolution_clock::now();
      timeOT = time3 - time2;

      std::vector<SIMDWireLabel> outputs = computeFunction(aliceInput, bobInput, e);

      Log::v("GC", "compute done");

      auto time4 = std::chrono::high_resolution_clock::now();
      timeEval = time4 - time3;

      Log::v("GC", "numANDs: %zu; numXORs: %zu", e.getNumANDs(), e.getNumXORs());

      std::vector<BitVector> output = e.outputToBob(outputs);
      auto time5 = std::chrono::high_resolution_clock::now();
      timeOutput = time5 - time4;
      Log::v("GC", "output done");

      return output;
    }

    std::vector<BitVector> SIMDCircuit::evaluateSIMD(
        const std::vector<BitVector> &inputB) {
      size_t transfer;
      channel.recv((uint8_t *)&transfer, sizeof(transfer));
      transfer = be64toh(transfer);
      const size_t SIMDvalues = inputB.size();
      Log::v("GC", "SIMD: %zu, transfer:%zu", SIMDvalues, transfer);
      assert(SIMDvalues == transfer);

      SIMDEvaluator e(channel, SIMDvalues);
      auto time1 = std::chrono::high_resolution_clock::now();

      e.performBaseOTs();
      Log::v("GC", "baseOTs done");

      auto time2 = std::chrono::high_resolution_clock::now();
      timeBaseOT = time2 - time1;

      assert(inputB.front().size() == mInputB_size);
      std::vector<SIMDWireLabel> aliceInput = e.inputOfAliceSIMD(mInputA_size);
      Log::v("GC", "inputA done");

      Log::v("GC", "inputA sent: %zu, recv: %zu", channel.getBytesSent(),
             channel.getBytesRecv());
      std::vector<SIMDWireLabel> bobInput = e.inputOfBob(inputB);

      Log::v("GC", "inputB done");

      auto time3 = std::chrono::high_resolution_clock::now();
      timeOT = time3 - time2;

      std::vector<SIMDWireLabel> outputs = computeFunction(aliceInput, bobInput, e);

      Log::v("GC", "compute done");

      auto time4 = std::chrono::high_resolution_clock::now();
      timeEval = time4 - time3;

      Log::v("GC", "numANDs: %zu", e.getNumANDs());
      Log::v("GC", "numXORs: %zu", e.getNumXORs());

      std::vector<BitVector> output = e.outputToBob(outputs);
      auto time5 = std::chrono::high_resolution_clock::now();
      timeOutput = time5 - time4;
      Log::v("GC", "output done");

      return output;
    }

    //----------------------------------------------------------------------------------------------------------------------

    void SIMDCircuitPhases::garbleBase(const BitVector &inputA,
                                       const size_t SIMDvalues) {
        g = new SIMDGarblerPhases(channel, SIMDvalues);
        auto time1 = std::chrono::high_resolution_clock::now();

        g->performBaseOTs();
        auto time2 = std::chrono::high_resolution_clock::now();
        timeBaseOT = time2 - time1;
        g->doOTPhase(mInputB_size * SIMDvalues);
        auto time3 = std::chrono::high_resolution_clock::now();
        timeOT = time3 - time2;

        assert(inputA.size() == mInputA_size);
        // build GC into bufChan
        std::vector<WireLabel> aliceInput = g->inputOfAlice(inputA);
        std::vector<SIMDWireLabel> bobInput = g->inputOfBobOffline(mInputB_size);
        std::vector<SIMDWireLabel> outputs = computeFunction(aliceInput, bobInput, *g);
        g->outputToBob(outputs);
        auto time4 = std::chrono::high_resolution_clock::now();
        timeEval = time4 - time3;

        std::vector<uint8_t> gcs = g->bufChan.getBuffer();
        uint64_t gc_size = gcs.size();
        size_t transfer;
        transfer = htobe64(gc_size);
        channel.send((uint8_t *)&transfer, sizeof(transfer));
        time4 = std::chrono::high_resolution_clock::now();
        channel.send(gcs.data(), gc_size);

        Log::v("GC", "Base comm: %fMiB sent, %fMiB recv",
             channel.getBytesSent() / 1024.0 / 1024.0,
             channel.getBytesRecv() / 1024.0 / 1024.0);
        Log::v("GC", "size of GCs: %zu bytes", gc_size);
            channel.clearStats();
        auto time5 = std::chrono::high_resolution_clock::now();
        timeSendGC = time5 - time4;
        Log::v("GC", "Base phase: %fsec, send: %fsec, total %fsec",
             std::chrono::duration<double>(time4 - time1).count(),
             timeSendGC.count(),
             std::chrono::duration<double>(time5 - time1).count());
    }

    void SIMDCircuitPhases::garbleOnline() {
      auto time1 = std::chrono::high_resolution_clock::now();
      g->inputOfBobOnline();
      auto time2 = std::chrono::high_resolution_clock::now();
      timeOnline = time2 - time1;

      Log::v("GC", "Online comm: %fKiB sent, %fKiB recv",
             channel.getBytesSent() / 1024.0, channel.getBytesRecv() / 1024.0);
      channel.clearStats();
    }

    void SIMDCircuitPhases::evaluateBase(size_t SIMDvalues) {
      e = new SIMDEvaluatorPhases(channel, SIMDvalues);
      auto time1 = std::chrono::high_resolution_clock::now();

      e->performBaseOTs();
      auto time2 = std::chrono::high_resolution_clock::now();
      timeBaseOT = time2 - time1;

      PRNG p = PRNG::getTestPRNG();
      randChoices_.reset(SIMDvalues * mInputB_size);
      randChoices_.randomize(p);
      e->doOTPhase(randChoices_);
      //        Log::v("GC", "baseOTs done");
      auto time3 = std::chrono::high_resolution_clock::now();
      timeOT = time3 - time2;

      size_t transfer;
      channel.recv((uint8_t *)&transfer, sizeof(transfer));
      time3 = std::chrono::high_resolution_clock::now();
      uint64_t gc_size = be64toh(transfer);

      std::vector<uint8_t> gcs(gc_size);
      channel.recv(gcs.data(), gcs.size());
      e->bufChan.setBuffer(gcs);

      auto time4 = std::chrono::high_resolution_clock::now();
      timeSendGC = time4 - time3;
    }

    std::vector<BitVector> SIMDCircuitPhases::evaluateOnline(
        const std::vector<BitVector> &inputB) {
      auto time4 = std::chrono::high_resolution_clock::now();

      assert(inputB.front().size() == mInputB_size);
      std::vector<WireLabel> aliceInput = e->inputOfAlice(mInputA_size);
      //        Log::v("GC", "inputA done");

      std::vector<SIMDWireLabel> bobInput =
          e->inputOfBobOnline(inputB, randChoices_);

      //        Log::v("GC", "inputB done");

      std::vector<SIMDWireLabel> outputs =
          computeFunction(aliceInput, bobInput, *e);

      //        Log::v("GC", "compute done");

      std::vector<BitVector> output = e->outputToBob(outputs);
      //        Log::v("GC", "output done");

      auto time5 = std::chrono::high_resolution_clock::now();
      timeEval = time5 - time4;

      return output;
    }
    }  // namespace droidCrypto
