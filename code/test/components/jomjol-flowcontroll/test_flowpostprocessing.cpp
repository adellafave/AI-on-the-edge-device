#include <unity.h>
#include <ClassFlowPostProcessing.h>
#include <ClassFlowCNNGeneral.h>
#include <ClassFlowCNNGeneral.h>
#include <ClassFlowMakeImage.h>

void setUpClassFlowPostprocessing(void);
string process_doFlow(std::vector<float> analog, std::vector<float> digits, t_CNNType digType = Digital100, 
                bool checkConsistency=false,  bool extendedResolution=false, int decimal_shift=0);

ClassFlowCNNGeneral* _analog;
ClassFlowCNNGeneral* _digit;
std::vector<ClassFlow*> FlowControll;
ClassFlowMakeImage* flowmakeimage;


class UnderTestPost : public ClassFlowPostProcessing {
    public:
        UnderTestPost(std::vector<ClassFlow*>* lfc, ClassFlowCNNGeneral *_analog, ClassFlowCNNGeneral *_digit)
            : ClassFlowPostProcessing::ClassFlowPostProcessing(lfc, _analog, _digit) {}
        using ClassFlowPostProcessing::InitNUMBERS;
};

UnderTestPost* undertestPost;


/**
 * @brief Testet die doFlow-Methode von ClassFlowPostprocessing
 * digits[] - enthält die liste der vom Model zurückgegebenen Ergebnisse (class100/cont) in der Reihenfolge von links nach rechts
 * analog[] - enthält die Liste der Zeiger vom Model, wie bei den digits
 * expected - enthält das erwartete Ergebnis, wobei der Dezimalpunkt genau zwischen digits und analog ist.
 * 
 */
void test_doFlow() {
        /*
         * 
         * digit1 = 1.2
         * digit2 = 6.7
         * analog1 = 9.5
         * analog2 = 8.4
         * 
         * Das Ergebnis sollte "16.984" sein. Bzw. 16.98 ohne Extended true
         */
        std::vector<float> digits = { 1.2, 6.7};
        std::vector<float> analogs = { 9.5, 8.4};
        const char* expected = "16.98";
        const char* expected_extended = "16.984";
        std::string result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921
         * 
         * Das Ergebnis sollte "376529.6" sein. 
         */
        digits = { 3.0, 7.0, 6.0, 5.0, 2.5, 9.6};
        analogs = { 6.4};
        expected = "376529.6";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921
         * 
         * Das Ergebnis sollte "167734.6" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 1.1, 6.0, 7.0, 7.0, 3.0, 4.6};
        analogs = { 6.2};
        expected = "167734.6";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/919
         * 
         * Das Ergebnis sollte "58.96889" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 5.0, 8.6};
        analogs = { 9.8, 6.7, 8.9, 8.6, 9.8};
        expected = "58.96889";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
  
        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921
         * 
         * Das Ergebnis sollte "376529.6" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 2.9, 7.0, 6.8, 9.9, 8.0, 3.9};
        analogs = { 9.7};
        expected = "377083.9";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        digits = { 1.1, 9.0, 4.0};
        analogs = { 6.1, 2.6, 6.25, 9.7};
        expected = "194.6259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        digits = { 1.1, 9.0, 4.0};
        analogs = { 8.1, 2.6, 6.25, 9.7};
        expected = "194.8259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        digits = { 1.1, 9.0, 4.0};
        analogs = { 9.1, 2.6, 6.25, 9.7};
        expected = "193.9259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/discussions/950
        digits = { 1.0, 9.0, 9.0};
        analogs = { 7.1, 4.8, 8.3};
        expected = "199.748";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/948
        digits = { 1.0, 9.0, 9.0};
        analogs = { 7.1, 4.8, 8.3};
        expected = "199.748";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
       
        // https://github.com/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1226966346
        digits = { 0.0, 2.9, 3.0, 2.9, 3.5, 9.5};
        analogs = {        };
        expected = "33330";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1226966346
        digits = { 9.9, 2.8, 2.9, 2.9, 3.7, 9.7};
        analogs = {        };
        expected = "33340";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/942
        digits = { 0.0, 9.9, 6.8, 9.9, 3.7, 0.8, 6.9, 8.7};
        analogs = {        };
        expected = "704179";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1228343319
        digits = { 9.9, 6.8, 1.1, 4.7, 2.7, 6.0, 9.0, 2.8};  // changed 3.7 --> 2.7 (see picture in issue)
        analogs = {        };
        expected = "7153693";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // Analoger Übergang Zähler Jomjolcom/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1228343319
        digits = { 1.0, 9.0, 4.3};  // changed 3.7 --> 2.7 (see picture in issue)
        analogs = { 8.9, 0.7, 8.9, 9.4 };
        expected = "194.9089";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei V11.2.0
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1229552041
        digits = { 2.9, 7.0, 7.0, 9.1, 8.1, 8.5};  // 376.9884(1) als falsches Ergebnis
        analogs = { 4.1 };
        expected = "377988.4";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei V11.2.0
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1233149877
        digits = { 0.0, 0.0, 7.0, 8.9};  // 79.9999(6) als falsches Ergebnis
        analogs = { 0.1, 0.1, 0.1, 9.6};
        expected = "78.9999";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1236119370
        digits = { 3.1, 9.1, 5.7};  // 9.1 führt zu falscher Erkennung eines unvollständigen Übergangs
        analogs = { 8.8, 6.1, 3.0, 2.0};
        expected = "395.8632";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/discussions/950#discussion-4338615
        digits = { 1.0, 9.0, 9.0};  // Übergang wurde um 1 erhöht (200, statt 199)
        analogs = { 7.1, 4.8, 8.3};
        expected = "199.748";
        result = process_doFlow(analogs, digits, Digital);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei Rolling (2002-09-09)
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1242730397
        digits = { 3.0, 2.0, 2.0, 8.0, 9.0, 4.0, 1.7, 9.8};  // falscher Wert 32290.420
        analogs = { };
        expected = "32289.420";
        expected_extended= "32289.4198";
        // FALSCH! wegen ungenügender Präzision von NUMBERS->Value
        // expected_extended= "32289.4198";

        // extendResolution=false, checkConsistency=false
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // checkConsistency=true und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // Fehler Rolling (2022-09-10)
        // not documented as issue
        digits = { 0.0, 0.0, 7.9, 3.8};  // 84.99401 als falsches Ergebnis
        analogs = { 0.0, 9.4, 4.1, 0.1};
        expected = "83.9940";
        expected_extended= "83.99401";

        // checkConsistency=false
        result = process_doFlow(analogs, digits, Digital100, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, true);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // checkConsistency=true und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // Fehler Rolling (2022-09-10)
        // https://github.com/jomjol/AI-on-the-edge-device/issues/994#issue-1368570945
        digits = { 0.0, 0.0, 1.0, 2.0, 2.8, 1.9, 2.8, 5.6};  // 123245.6 als falsches Ergebnis
        analogs = { };
        expected = "123236";
        expected_extended= "123235.6";
        
        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, false, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, true, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/discussions/950#discussioncomment-3661982
        digits = { 3.0, 2.0, 4.1, 9.0, 4.0, 6.3, 9.2};  // 3249.459 als falsches Ergebnis
        analogs = { };
        expected = "3249.469";
        expected_extended= "3249.4692";

        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // checkConsistency=true
        // checkConsistency NOT working correctly
        //result = process_doFlow(analogs, digits, Digital100, true, false, -3);
        //TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1020#issue-1375648891
        digits = { 0.0, 2.0, 6.1, 9.2};  // 259.9227 als falsches Ergebnis
        analogs = { 9.0, 2.5, 2.9, 7.2};
        expected = "269.9227";
        expected_extended= "269.92272";
        // Float Value reduziert die Genauigkeit hier. Korrekt wäre
        // expected_extended= "269.92272";

        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, false, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // checkConsistency=true
        // checkConsistency NOT working correctly
        //result = process_doFlow(analogs, digits, Digital100, true, false, -3);
        //TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // checkConsistency=true und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

}



void setUpClassFlowPostprocessing(t_CNNType digType, t_CNNType anaType)
{
    
    // wird im doFlow verwendet
    flowmakeimage = new ClassFlowMakeImage(&FlowControll);
    FlowControll.push_back(flowmakeimage);

    // Die Modeltypen werden gesetzt, da keine Modelle verwendet werden.
    _analog = new ClassFlowCNNGeneral(nullptr, anaType);
    
    _digit =  new ClassFlowCNNGeneral(nullptr, digType);

    undertestPost = new UnderTestPost(&FlowControll, _analog, _digit);
  
}


std::string process_doFlow(std::vector<float> analog, std::vector<float> digits, t_CNNType digType, 
            bool checkConsistency, bool extendedResolution, int decimal_shift) {
    // setup the classundertest
    setUpClassFlowPostprocessing(digType, Analogue100);

    printf("SetupClassFlowPostprocessing completed.\n");

    // digits
    if (digits.size()>0) {
        general* gen_digit = _digit->GetGENERAL("default", true);
        gen_digit->ROI.clear();
        for (int i = 0; i<digits.size(); i++) {
            roi* digitROI = new roi();
            string name = "digit_" + std::to_string(i);
            digitROI->name = name;
            digitROI->result_klasse = (int) digits[i];
            digitROI->result_float = digits[i];
            gen_digit->ROI.push_back(digitROI);
        }
    }

    // analog
    if (analog.size()>0) {
        general* gen_analog = _analog->GetGENERAL("default", true);
        gen_analog->ROI.clear();

        for (int i = 0; i<analog.size(); i++) {
            roi* anaROI = new roi();
            string name = "ana_" + std::to_string(i);
            anaROI->name = name;
            anaROI->result_float = analog[i];
            gen_analog->ROI.push_back(anaROI);
        }
    }
    printf("Setup ROIs completed.\n");

    undertestPost->InitNUMBERS();
    if (checkConsistency) {
        printf("checkConsistency=true\n");
        std::vector<NumberPost*>* NUMBERS = undertestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            printf("Set checkConsistency on number: %d\n", _n);
            (*NUMBERS)[_n]->checkDigitIncreaseConsistency = true;
        }
    }
    if (extendedResolution ) {
       std::vector<NumberPost*>* NUMBERS = undertestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            printf("Set extendedResolution on number: %d\n", _n);
            (*NUMBERS)[_n]->isExtendedResolution = true;
        }

    }
    if (decimal_shift!=0) {
        std::vector<NumberPost*>* NUMBERS = undertestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            printf("Set decimalshif on number: %d to %d\n", _n, decimal_shift);
            (*NUMBERS)[_n]->DecimalShift = decimal_shift;
            (*NUMBERS)[_n]->DecimalShiftInitial = decimal_shift;   
        }       
    }

    string time;
    // run test
    TEST_ASSERT_TRUE(undertestPost->doFlow(time));

    return undertestPost->getReadout(0);

}

