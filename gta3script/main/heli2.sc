MISSION_START
// *******************************************************************************************
// ************************************* heli 2 **********************************************
// *******************************************************************************************
// *******************************************************************************************
// *** Simple Check Point Race in a Chopper												   ***
// *******************************************************************************************

// Mission start stuff

GOSUB mission_start_heli2

IF HAS_DEATHARREST_BEEN_EXECUTED
	GOSUB mission_heli2_failed
ENDIF

GOSUB mission_cleanup_heli2

MISSION_END


// Variables for mission


//LOAD_AND_LAUNCH_MISSION_EXCLUSIVE var_ints
//VAR_INT	heli heli_blip heli_2nd_blip heli_cpcounter heli_timer
//VAR_FLOAT heli_cp_x heli_cp_y heli_cp_z heli_cp_2nd_x heli_cp_2nd_y heli_cp_2nd_z
//VAR_INT mins seconds temp_int
//VAR_FLOAT cp0_x cp0_y cp0_z
//VAR_FLOAT cp1_x cp1_y cp1_z
//VAR_FLOAT cp2_x cp2_y cp2_z
//VAR_FLOAT cp3_x cp3_y cp3_z
//VAR_FLOAT cp4_x cp4_y cp4_z
//VAR_FLOAT cp5_x cp5_y cp5_z
//VAR_FLOAT cp6_x cp6_y cp6_z
//VAR_FLOAT cp7_x cp7_y cp7_z
//VAR_FLOAT cp8_x cp8_y cp8_z
//VAR_FLOAT cp9_x cp9_y cp9_z
//VAR_FLOAT cp10_x cp10_y cp10_z
//VAR_FLOAT cp11_x cp11_y cp11_z
//VAR_FLOAT cp12_x cp12_y cp12_z
//VAR_FLOAT cp13_x cp13_y cp13_z
//VAR_FLOAT cp14_x cp14_y cp14_z
//VAR_FLOAT cp15_x cp15_y cp15_z
//VAR_FLOAT cp16_x cp16_y cp16_z
//VAR_FLOAT cp17_x cp17_y cp17_z
//VAR_FLOAT cp18_x cp18_y cp18_z
//VAR_FLOAT cp19_x cp19_y cp19_z
//VAR_FLOAT cp20_x cp20_y cp20_z
//VAR_FLOAT cp21_x cp21_y cp21_z
//VAR_FLOAT cp22_x cp22_y cp22_z
//VAR_FLOAT cp23_x cp23_y cp23_z
//VAR_FLOAT cp24_x cp24_y cp24_z
//VAR_FLOAT cp25_x cp25_y cp25_z
//VAR_FLOAT cp26_x cp26_y cp26_z
//VAR_FLOAT cp27_x cp27_y cp27_z
//VAR_FLOAT cp28_x cp28_y cp28_z
//VAR_FLOAT cp29_x cp29_y cp29_z
//VAR_FLOAT cp30_x cp30_y cp30_z
//VAR_FLOAT cp31_x cp31_y cp31_z
//VAR_FLOAT cp32_x cp32_y cp32_z

// ***************************************Mission Start*************************************

mission_start_heli2:

flag_player_on_mission = 1

heli_cpcounter = 0

SCRIPT_NAME heli2sc

WAIT 0

IF done_heli2_progress	= 0
	REGISTER_MISSION_GIVEN
ENDIF
//28.4463    //cargen
//-1311.7614 //cargen
//16.4712    //cargen

cp1_x =  77.8349 
cp1_y =  -1224.6879
cp1_z =  17.9498 

cp2_x =  103.4742 
cp2_y =  -1127.8851
cp2_z =  18.1870 

cp3_x =  203.3635 
cp3_y =  -1135.5078
cp3_z =  21.5310 

cp4_x =  260.3221 
cp4_y =  -1217.4844
cp4_z =  13.0769 

cp5_x =  164.5704 
cp5_y =  -1245.7279
cp5_z =  21.9229 

cp6_x =  70.0126 
cp6_y =  -1278.6166
cp6_z =  19.1218 

cp7_x =  37.6583 
cp7_y =  -1373.4507
cp7_z =  19.4852 

cp8_x =  -61.5636 
cp8_y =  -1381.0027
cp8_z =  30.3879 

cp9_x =  -155.5676 
cp9_y =  -1349.1412
cp9_z =  17.5384 

cp10_x = -185.4706 
cp10_y = -1444.6370
cp10_z = 14.7326 

cp11_x = -113.9027 
cp11_y = -1513.1001
cp11_z = 29.4323 

cp12_x = -27.0724 
cp12_y = -1561.8441
cp12_z = 19.2416 

cp13_x = 52.9751 
cp13_y = -1502.7623
cp13_z = 32.7119 

cp14_x = 128.6115 
cp14_y = -1438.5127
cp14_z = 19.4189 

cp15_x = 178.8131 
cp15_y = -1351.6604
cp15_z = 23.4365 

cp16_x = 140.4057 
cp16_y = -1259.0345
cp16_z = 23.2808 

cp17_x = 49.4630 
cp17_y = -1300.5123
cp17_z = 18.1885 

GOSUB get_next_heli2_checkpoint

PRINT_NOW HELI_1A 15000 1//"Test your skills with the Sparrow, see how quickly you can complete the course."

ADD_BLIP_FOR_COORD heli_cp_x heli_cp_y heli_cp_z heli_blip
CHANGE_BLIP_COLOUR heli_blip PURPLE
CHANGE_BLIP_DISPLAY heli_blip BLIP_ONLY
CHANGE_BLIP_SCALE heli_blip 3
ADD_BLIP_FOR_COORD heli_cp_2nd_x heli_cp_2nd_y heli_cp_2nd_z heli_2nd_blip
CHANGE_BLIP_COLOUR heli_2nd_blip PURPLE 
CHANGE_BLIP_DISPLAY heli_2nd_blip BLIP_ONLY
CHANGE_BLIP_SCALE heli_2nd_blip 2
DIM_BLIP heli_2nd_blip TRUE

heli_timer = 0

DISPLAY_ONSCREEN_TIMER_WITH_STRING heli_timer TIMER_UP RACES

IF IS_PLAYER_IN_ANY_CAR player1
	STORE_CAR_PLAYER_IS_IN player1 heli
ELSE
	GOTO mission_heli2_failed
ENDIF

WHILE NOT heli_cpcounter = 17
	WAIT 0
	
	IF IS_CAR_DEAD heli
		GOTO mission_heli2_failed
	ENDIF
	
	IF NOT IS_PLAYER_IN_CAR player1 heli
		GOTO mission_heli2_failed
	ENDIF

	IF LOCATE_PLAYER_IN_CAR_3D player1 heli_cp_x heli_cp_y heli_cp_z 5.0 5.0 5.0 0
		
		ADD_ONE_OFF_SOUND 0.0 0.0 0.0 SOUND_PART_MISSION_COMPLETE
		++ heli_cpcounter
		
		REMOVE_BLIP heli_blip
		REMOVE_BLIP heli_2nd_blip
		
		GOSUB get_next_heli2_checkpoint
		
		ADD_BLIP_FOR_COORD heli_cp_x heli_cp_y heli_cp_z heli_blip
		CHANGE_BLIP_COLOUR heli_blip PURPLE
		CHANGE_BLIP_DISPLAY heli_blip BLIP_ONLY
		CHANGE_BLIP_SCALE heli_blip 3

		IF heli_cpcounter < 16
			ADD_BLIP_FOR_COORD heli_cp_2nd_x heli_cp_2nd_y heli_cp_2nd_z heli_2nd_blip
			CHANGE_BLIP_DISPLAY heli_2nd_blip BLIP_ONLY
			CHANGE_BLIP_SCALE heli_2nd_blip 2
			DIM_BLIP heli_2nd_blip TRUE
			IF heli_cpcounter < 15
				CHANGE_BLIP_COLOUR heli_2nd_blip PURPLE 
			ELSE
				CHANGE_BLIP_COLOUR heli_2nd_blip RED 
			ENDIF
		ELSE
			CHANGE_BLIP_COLOUR heli_blip RED
		ENDIF

	ENDIF

	IF heli_cpcounter = 16
		DRAW_WEAPONSHOP_CORONA heli_cp_x heli_cp_y heli_cp_z 4.5 CORONATYPE_CIRCLE FLARETYPE_NONE 200 0 0
	ELSE
		DRAW_WEAPONSHOP_CORONA heli_cp_x heli_cp_y heli_cp_z 4.5 CORONATYPE_CIRCLE FLARETYPE_NONE 200 0 200
	ENDIF

ENDWHILE

GOTO mission_heli2_passed


 // Mission heli2 failed

mission_heli2_failed:
PRINT_NOW HELI_F1 5000 1 //"HELI MISSION CANCELLED"
RETURN

   

// mission heli2 passed

mission_heli2_passed:

PRINT_WITH_NUMBER_BIG HELI_1B 100 5000 1 //"Course Complete!"
ADD_SCORE player1 100
CLEAR_WANTED_LEVEL player1
PLAY_MISSION_PASSED_TUNE 1

IF done_heli2_progress	= 0
	PLAYER_MADE_PROGRESS 1
	REGISTER_ODDJOB_MISSION_PASSED
	done_heli2_progress	= 1
ENDIF

heli_timer = heli_timer / 1000
REGISTER_FASTEST_TIME 13 heli_timer

mins = heli_timer / 60
temp_int = mins * 60
seconds = heli_timer - temp_int

IF heli2_checkpoint_best_time > heli_timer
	heli2_checkpoint_best_time = heli_timer
	IF seconds > 9
		PRINT_WITH_2_NUMBERS_NOW RACES17 mins seconds 10000 1 //"New best time: ~1~:~1~"
	ELSE
		PRINT_WITH_2_NUMBERS_NOW RACES20 mins seconds 10000 1 //"New best time: ~1~:0~1~"
	ENDIF
ELSE
	IF seconds > 9
		PRINT_WITH_2_NUMBERS_NOW RACES_9 mins seconds 10000 1 //"Time: ~1~:~1~"
	ELSE
		PRINT_WITH_2_NUMBERS_NOW RACES21 mins seconds 10000 1 //"Time: ~1~:0~1~"
	ENDIF
ENDIF

RETURN
		


// mission cleanup

mission_cleanup_heli2:

flag_player_on_mission = 0

GET_GAME_TIMER timer_mobile_start
CLEAR_ONSCREEN_TIMER heli_timer
REMOVE_BLIP	heli_blip
REMOVE_BLIP	heli_2nd_blip

MISSION_HAS_FINISHED
RETURN



///////////////////////////////////////////////
get_next_heli2_checkpoint://////////////////////
///////////////////////////////////////////////

	IF heli_cpcounter = 0
		heli_cp_x = cp1_x
		heli_cp_y = cp1_y
		heli_cp_z = cp1_z
		heli_cp_2nd_x = cp2_x
		heli_cp_2nd_y = cp2_y
		heli_cp_2nd_z = cp2_z
	ENDIF

	IF heli_cpcounter = 1
		heli_cp_x = cp2_x
		heli_cp_y = cp2_y
		heli_cp_z = cp2_z
		heli_cp_2nd_x = cp3_x
		heli_cp_2nd_y = cp3_y
		heli_cp_2nd_z = cp3_z
	ENDIF

	IF heli_cpcounter = 2
		heli_cp_x = cp3_x
		heli_cp_y = cp3_y
		heli_cp_z = cp3_z
		heli_cp_2nd_x = cp4_x
		heli_cp_2nd_y = cp4_y
		heli_cp_2nd_z = cp4_z
	ENDIF

	IF heli_cpcounter = 3
		heli_cp_x = cp4_x
		heli_cp_y = cp4_y
		heli_cp_z = cp4_z
		heli_cp_2nd_x = cp5_x
		heli_cp_2nd_y = cp5_y
		heli_cp_2nd_z = cp5_z
	ENDIF

	IF heli_cpcounter = 4
		heli_cp_x = cp5_x
		heli_cp_y = cp5_y
		heli_cp_z = cp5_z
		heli_cp_2nd_x = cp6_x
		heli_cp_2nd_y = cp6_y
		heli_cp_2nd_z = cp6_z
	ENDIF

	IF heli_cpcounter = 5
		heli_cp_x = cp6_x
		heli_cp_y = cp6_y
		heli_cp_z = cp6_z
		heli_cp_2nd_x = cp7_x
		heli_cp_2nd_y = cp7_y
		heli_cp_2nd_z = cp7_z
	ENDIF

	IF heli_cpcounter = 6
		heli_cp_x = cp7_x
		heli_cp_y = cp7_y
		heli_cp_z = cp7_z
		heli_cp_2nd_x = cp8_x
		heli_cp_2nd_y = cp8_y
		heli_cp_2nd_z = cp8_z
	ENDIF

	IF heli_cpcounter = 7
		heli_cp_x = cp8_x
		heli_cp_y = cp8_y
		heli_cp_z = cp8_z
		heli_cp_2nd_x = cp9_x
		heli_cp_2nd_y = cp9_y
		heli_cp_2nd_z = cp9_z
	ENDIF

	IF heli_cpcounter = 8
		heli_cp_x = cp9_x
		heli_cp_y = cp9_y
		heli_cp_z = cp9_z
		heli_cp_2nd_x = cp10_x
		heli_cp_2nd_y = cp10_y
		heli_cp_2nd_z = cp10_z
	ENDIF

	IF heli_cpcounter = 9
		heli_cp_x = cp10_x
		heli_cp_y = cp10_y
		heli_cp_z = cp10_z
		heli_cp_2nd_x = cp11_x
		heli_cp_2nd_y = cp11_y
		heli_cp_2nd_z = cp11_z
	ENDIF

	IF heli_cpcounter = 10
		heli_cp_x = cp11_x
		heli_cp_y = cp11_y
		heli_cp_z = cp11_z
		heli_cp_2nd_x = cp12_x
		heli_cp_2nd_y = cp12_y
		heli_cp_2nd_z = cp12_z
	ENDIF

	IF heli_cpcounter = 11
		heli_cp_x = cp12_x
		heli_cp_y = cp12_y
		heli_cp_z = cp12_z
		heli_cp_2nd_x = cp13_x
		heli_cp_2nd_y = cp13_y
		heli_cp_2nd_z = cp13_z
	ENDIF

	IF heli_cpcounter = 12
		heli_cp_x = cp13_x
		heli_cp_y = cp13_y
		heli_cp_z = cp13_z
		heli_cp_2nd_x = cp14_x
		heli_cp_2nd_y = cp14_y
		heli_cp_2nd_z = cp14_z
	ENDIF

	IF heli_cpcounter = 13
		heli_cp_x = cp14_x
		heli_cp_y = cp14_y
		heli_cp_z = cp14_z
		heli_cp_2nd_x = cp15_x
		heli_cp_2nd_y = cp15_y
		heli_cp_2nd_z = cp15_z
	ENDIF

	IF heli_cpcounter = 14
		heli_cp_x = cp15_x
		heli_cp_y = cp15_y
		heli_cp_z = cp15_z
		heli_cp_2nd_x = cp16_x
		heli_cp_2nd_y = cp16_y
		heli_cp_2nd_z = cp16_z
	ENDIF

	IF heli_cpcounter = 15
		heli_cp_x = cp16_x
		heli_cp_y = cp16_y
		heli_cp_z = cp16_z
		heli_cp_2nd_x = cp17_x
		heli_cp_2nd_y = cp17_y
		heli_cp_2nd_z = cp17_z
	ENDIF

	IF heli_cpcounter = 16
		heli_cp_x = cp17_x
		heli_cp_y = cp17_y
		heli_cp_z = cp17_z
		heli_cp_2nd_x = cp18_x
		heli_cp_2nd_y = cp18_y
		heli_cp_2nd_z = cp18_z
	ENDIF

	IF heli_cpcounter = 17
		heli_cp_x = cp18_x
		heli_cp_y = cp18_y
		heli_cp_z = cp18_z
		heli_cp_2nd_x = cp19_x
		heli_cp_2nd_y = cp19_y
		heli_cp_2nd_z = cp19_z
	ENDIF

	IF heli_cpcounter = 18
		heli_cp_x = cp19_x
		heli_cp_y = cp19_y
		heli_cp_z = cp19_z
		heli_cp_2nd_x = cp20_x
		heli_cp_2nd_y = cp20_y
		heli_cp_2nd_z = cp20_z
	ENDIF

	IF heli_cpcounter = 19
		heli_cp_x = cp20_x
		heli_cp_y = cp20_y
		heli_cp_z = cp20_z
		heli_cp_2nd_x = cp21_x
		heli_cp_2nd_y = cp21_y
		heli_cp_2nd_z = cp21_z
	ENDIF

	IF heli_cpcounter = 20
		heli_cp_x = cp21_x
		heli_cp_y = cp21_y
		heli_cp_z = cp21_z
		heli_cp_2nd_x = cp22_x
		heli_cp_2nd_y = cp22_y
		heli_cp_2nd_z = cp22_z
	ENDIF

	IF heli_cpcounter = 21
		heli_cp_x = cp22_x
		heli_cp_y = cp22_y
		heli_cp_z = cp22_z
		heli_cp_2nd_x = cp23_x
		heli_cp_2nd_y = cp23_y
		heli_cp_2nd_z = cp23_z
	ENDIF

	IF heli_cpcounter = 22
		heli_cp_x = cp23_x
		heli_cp_y = cp23_y
		heli_cp_z = cp23_z
		heli_cp_2nd_x = cp24_x
		heli_cp_2nd_y = cp24_y
		heli_cp_2nd_z = cp24_z
	ENDIF

	IF heli_cpcounter = 23
		heli_cp_x = cp24_x
		heli_cp_y = cp24_y
		heli_cp_z = cp24_z
		heli_cp_2nd_x = cp25_x
		heli_cp_2nd_y = cp25_y
		heli_cp_2nd_z = cp25_z
	ENDIF

	IF heli_cpcounter = 24
		heli_cp_x = cp25_x
		heli_cp_y = cp25_y
		heli_cp_z = cp25_z
		heli_cp_2nd_x = cp26_x
		heli_cp_2nd_y = cp26_y
		heli_cp_2nd_z = cp26_z
	ENDIF

	IF heli_cpcounter = 25
		heli_cp_x = cp26_x
		heli_cp_y = cp26_y
		heli_cp_z = cp26_z
		heli_cp_2nd_x = cp27_x
		heli_cp_2nd_y = cp27_y
		heli_cp_2nd_z = cp27_z
	ENDIF

	IF heli_cpcounter = 26
		heli_cp_x = cp27_x
		heli_cp_y = cp27_y
		heli_cp_z = cp27_z
		heli_cp_2nd_x = cp28_x
		heli_cp_2nd_y = cp28_y
		heli_cp_2nd_z = cp28_z
	ENDIF

	IF heli_cpcounter = 27
		heli_cp_x = cp28_x
		heli_cp_y = cp28_y
		heli_cp_z = cp28_z
		heli_cp_2nd_x = cp29_x
		heli_cp_2nd_y = cp29_y
		heli_cp_2nd_z = cp29_z
	ENDIF

	IF heli_cpcounter = 28
		heli_cp_x = cp29_x
		heli_cp_y = cp29_y
		heli_cp_z = cp29_z
		heli_cp_2nd_x = cp29_x
		heli_cp_2nd_y = cp29_y
		heli_cp_2nd_z = cp29_z
	ENDIF
///////////////////////////////////////////////
RETURN/////////////////////////////////////////
///////////////////////////////////////////////
