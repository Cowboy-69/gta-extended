// Vice Extended (ViceEx)
// *****************************************************************************************
// ************************************Clothing store***************************************
// *****************************************************************************************

MISSION_START

VAR_INT clothing_store_flag // Activates or deactivates the clothing store
clothing_store_flag = 1

VAR_INT streetVariationState, soireeVariationState, countryVariationState, casualVariationState, vercettiVariationState, blackTracksuitVariationState, redTracksuitVariationState
// Four states (need for wardrobe):
// 0 - None of the clothing variations were purchased (default)
// 1 - Only the first variation was purchased
// 2 - Only the second variation was purchased
// 3 - All variations of clothing purchased
streetVariationState = 0
soireeVariationState = 0
countryVariationState = 0
casualVariationState = 0
vercettiVariationState = 0
blackTracksuitVariationState = 0
redTracksuitVariationState = 0

SCRIPT_NAME clothes

SET_DEATHARREST_STATE OFF

mission_start_clothes:
	START_NEW_SCRIPT shop_clothes

	MISSION_END

shop_clothes:

SCRIPT_NAME CL_ST

{

LVAR_INT bPlayerInMarker, bPlayerInStore
LVAR_INT storedClothingModel, currentClothingModel, storedVariation, currentVariation
LVAR_INT storedStoreClothingNumber currentStoreClothingNumber, maxStoreClothingNumbers, currentClothingPrice

bPlayerInMarker = FALSE
bPlayerInStore = FALSE
storedClothingModel = 1
currentClothingModel = 1
storedVariation = 0
currentVariation = 0
storedStoreClothingNumber = 1
currentStoreClothingNumber = 1
maxStoreClothingNumbers = 14
currentClothingPrice = 0

shop_clothes_loop:
    WAIT 0
    
    if (clothing_store_flag = 0)
        GOTO shop_clothes_loop
    endif

    if (bPlayerInStore = TRUE)
        GOTO player_in_store_loop
    endif

    if (IS_WANTED_LEVEL_GREATER player1 0)
    or (IS_CHAR_DEAD scplayer)
        GOTO shop_clothes_loop
    endif

    if (bPlayerInMarker = FALSE)
        //if (not LOCATE_CHAR_ON_FOOT_3D scplayer, 142.0, -821.0, 10.0, 1.3, 1.3, 1.3, TRUE) // debug
        if (not LOCATE_CHAR_ON_FOOT_3D scplayer, 414.0, 1038.0, 19.0, 1.3, 1.3, 1.3, TRUE)
            GOTO shop_clothes_loop
        endif
    else 
        if (bPlayerInMarker = 1)
            //if (not LOCATE_CHAR_ON_FOOT_3D scplayer, 142.0, -821.0, 10.0, 1.3, 1.3, 1.3, FALSE) // debug
            if (not LOCATE_CHAR_ON_FOOT_3D scplayer, 414.0, 1038.0, 19.0, 1.3, 1.3, 1.3, FALSE)
                bPlayerInMarker = FALSE
            endif

            GOTO shop_clothes_loop
        endif
    endif

    bPlayerInMarker = TRUE
    bPlayerInStore = TRUE
    SET_PLAYER_CONTROL 0 FALSE

    DO_FADE 1000 FADE_OUT
    WAIT 1000

    CLEAR_ALL_CHAR_ANIMS scplayer
    //SET_CHAR_COORDINATES scplayer, 142.0, -821.0, 8.5 // debug
    SET_CHAR_COORDINATES scplayer, 414.0, 1038.0, 17.5
    //SET_CHAR_HEADING scplayer, 180.0 // debug
    SET_CHAR_HEADING scplayer, 0.0

    //SET_FIXED_CAMERA_POSITION 142.0, -823.5, 10.5, 0.0, 0.0, 0.0 // debug
    SET_FIXED_CAMERA_POSITION 414.0, 1040.5, 19.5, 0.0, 0.0, 0.0
    POINT_CAMERA_AT_CHAR scplayer 15 2

    GET_CHAR_CLOTHING_VARIATION scplayer storedVariation
    currentVariation = storedVariation

    // Street
    if (IS_PLAYER_WEARING player1 PLAYER)
        storedClothingModel = 1

        if (storedVariation = 0)
        or (storedVariation = 1)
            storedStoreClothingNumber = 1
        else
            storedStoreClothingNumber = 2
        endif
    endif

    // Soiree
    if (IS_PLAYER_WEARING player1 PLAYER2)
        storedClothingModel = 2
        
        if (storedVariation = 0)
        or (storedVariation = 1)
            storedStoreClothingNumber = 3
        else
            storedStoreClothingNumber = 4
        endif
    endif

    // Country Club
    if (IS_PLAYER_WEARING player1 PLAYER4)
        storedClothingModel = 3
        
        if (storedVariation = 0)
        or (storedVariation = 1)
            storedStoreClothingNumber = 5
        else
            storedStoreClothingNumber = 6
        endif
    endif

    // Casual
    if (IS_PLAYER_WEARING player1 PLAYER8)
        storedClothingModel = 4
        
        if (storedVariation = 0)
        or (storedVariation = 1)
            storedStoreClothingNumber = 7
        else
            storedStoreClothingNumber = 8
        endif
    endif

    // Mr Vercetti
    if (IS_PLAYER_WEARING player1 PLAYER9)
        storedClothingModel = 5
        
        if (storedVariation = 0)
        or (storedVariation = 1)
            storedStoreClothingNumber = 9
        else
            storedStoreClothingNumber = 10
        endif
    endif

    // Black tracksuit
    if (IS_PLAYER_WEARING player1 PLAY10)
        storedClothingModel = 6
        
        if (storedVariation = 0)
        or (storedVariation = 1)
            storedStoreClothingNumber = 11
        else
            storedStoreClothingNumber = 12
        endif
    endif

    // Red tracksuit
    if (IS_PLAYER_WEARING player1 PLAY11)
        storedClothingModel = 7
        
        if (storedVariation = 0)
        or (storedVariation = 1)
            storedStoreClothingNumber = 13
        else
            storedStoreClothingNumber = 14
        endif
    endif

    if (storedVariation = 0)
        if not (IS_PLAYER_WEARING player1 PLAYER)
            UNDRESS_CHAR scplayer PLAYER
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 1
        currentStoreClothingNumber = 1
        currentVariation = 1
        SET_CHAR_CLOTHING_VARIATION scplayer 1
    else
        currentClothingModel = storedClothingModel
        currentStoreClothingNumber = storedStoreClothingNumber
    endif

    DO_FADE 1000 FADE_IN
    WAIT 1000

    GOTO shop_clothes_loop

player_in_store_loop:
    GOSUB display_info

    // Switching clothing variations
    LVAR_INT leftStick_X, unused
    GET_POSITION_OF_ANALOGUE_STICKS 0, leftStick_X, unused, unused, unused
    if (leftStick_X < -64)
        if (currentStoreClothingNumber = 1)
            currentStoreClothingNumber = maxStoreClothingNumbers
        else
            currentStoreClothingNumber--
        endif

        GOTO change_clothes
    else
        if (leftStick_X > 64)
            if (currentStoreClothingNumber = maxStoreClothingNumbers)
                currentStoreClothingNumber = 1
            else
                currentStoreClothingNumber++
            endif

            GOTO change_clothes
        endif
    endif
    
    // Clothing buying
    if (IS_BUTTON_PRESSED PAD1 CROSS) // PED_SPRINT
        GOSUB buy_clothes
    endif

    // Leave store
    if (not IS_BUTTON_PRESSED PAD1 TRIANGLE) // VEHICLE_ENTER_EXIT
        GOTO shop_clothes_loop
    endif

    while (IS_BUTTON_PRESSED PAD1 TRIANGLE) // VEHICLE_ENTER_EXIT
        WAIT 0
    endwhile

    GOTO leave_store

display_info:
    PRINT_HELP_FOREVER CLOTH_1

    // Displaying clothing name and price

    USE_TEXT_COMMANDS TRUE

    // Street
    if (currentStoreClothingNumber = 1)
    or (currentStoreClothingNumber = 2)
        currentClothingPrice = 20

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT1

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 0.58 1.2
        if (streetVariationState = 1)
        and (currentVariation = 1)
            DISPLAY_TEXT 320.0 425.0 CLOTH_3
        else
            if (streetVariationState = 2)
            and (currentVariation = 2)
                DISPLAY_TEXT 320.0 425.0 CLOTH_3
            else
                if (streetVariationState = 3)
                    DISPLAY_TEXT 320.0 425.0 CLOTH_3
                else
                    SET_TEXT_COLOUR 255 255 255 255
                    DISPLAY_TEXT_WITH_NUMBER 320.0 425.0 G_COST currentClothingPrice
                endif
            endif
        endif
    endif

    // Soiree
    if (currentStoreClothingNumber = 3)
    or (currentStoreClothingNumber = 4)
        currentClothingPrice = 100

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT2

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 0.58 1.2
        if (clothes2_created = 0)
            DISPLAY_TEXT 320.0 425.0 STOCK
        else
            if (soireeVariationState = 1)
            and (currentVariation = 1)
                DISPLAY_TEXT 320.0 425.0 CLOTH_3
            else
                if (soireeVariationState = 2)
                and (currentVariation = 2)
                    DISPLAY_TEXT 320.0 425.0 CLOTH_3
                else
                    if (soireeVariationState = 3)
                        DISPLAY_TEXT 320.0 425.0 CLOTH_3
                    else
                        SET_TEXT_COLOUR 255 255 255 255
                        DISPLAY_TEXT_WITH_NUMBER 320.0 425.0 G_COST currentClothingPrice
                    endif
                endif
            endif
        endif
    endif

    // Country Club
    if (currentStoreClothingNumber = 5)
    or (currentStoreClothingNumber = 6)
        currentClothingPrice = 40

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT4

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 0.58 1.2
        if (clothes4_created = 0)
            DISPLAY_TEXT 320.0 425.0 STOCK
        else
            if (countryVariationState = 1)
            and (currentVariation = 1)
                DISPLAY_TEXT 320.0 425.0 CLOTH_3
            else
                if (countryVariationState = 2)
                and (currentVariation = 2)
                    DISPLAY_TEXT 320.0 425.0 CLOTH_3
                else
                    if (countryVariationState = 3)
                        DISPLAY_TEXT 320.0 425.0 CLOTH_3
                    else
                        SET_TEXT_COLOUR 255 255 255 255
                        DISPLAY_TEXT_WITH_NUMBER 320.0 425.0 G_COST currentClothingPrice
                    endif
                endif
            endif
        endif
    endif

    // Casual
    if (currentStoreClothingNumber = 7)
    or (currentStoreClothingNumber = 8)
        currentClothingPrice = 30
        
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT8

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 0.58 1.2
        if (clothes7_created = 0)
            DISPLAY_TEXT 320.0 425.0 STOCK
        else
            if (casualVariationState = 1)
            and (currentVariation = 1)
                DISPLAY_TEXT 320.0 425.0 CLOTH_3
            else
                if (casualVariationState = 2)
                and (currentVariation = 2)
                    DISPLAY_TEXT 320.0 425.0 CLOTH_3
                else
                    if (casualVariationState = 3)
                        DISPLAY_TEXT 320.0 425.0 CLOTH_3
                    else
                        SET_TEXT_COLOUR 255 255 255 255
                        DISPLAY_TEXT_WITH_NUMBER 320.0 425.0 G_COST currentClothingPrice
                    endif
                endif
            endif
        endif
    endif

    // Mr Vercetti
    if (currentStoreClothingNumber = 9)
    or (currentStoreClothingNumber = 10)
        currentClothingPrice = 120
        
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT9

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 0.58 1.2
        if (clothes8_created = 0)
            DISPLAY_TEXT 320.0 425.0 STOCK
        else
            if (vercettiVariationState = 1)
            and (currentVariation = 1)
                DISPLAY_TEXT 320.0 425.0 CLOTH_3
            else
                if (vercettiVariationState = 2)
                and (currentVariation = 2)
                    DISPLAY_TEXT 320.0 425.0 CLOTH_3
                else
                    if (vercettiVariationState = 3)
                        DISPLAY_TEXT 320.0 425.0 CLOTH_3
                    else
                        SET_TEXT_COLOUR 255 255 255 255
                        DISPLAY_TEXT_WITH_NUMBER 320.0 425.0 G_COST currentClothingPrice
                    endif
                endif
            endif
        endif
    endif

    // Black tracksuit
    if (currentStoreClothingNumber = 11)
    or (currentStoreClothingNumber = 12)
        currentClothingPrice = 50
        
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT10

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 0.58 1.2
        if (clothes9_created = 0)
            DISPLAY_TEXT 320.0 425.0 STOCK
        else
            if (blackTracksuitVariationState = 1)
            and (currentVariation = 1)
                DISPLAY_TEXT 320.0 425.0 CLOTH_3
            else
                if (blackTracksuitVariationState = 2)
                and (currentVariation = 2)
                    DISPLAY_TEXT 320.0 425.0 CLOTH_3
                else
                    if (blackTracksuitVariationState = 3)
                        DISPLAY_TEXT 320.0 425.0 CLOTH_3
                    else
                        SET_TEXT_COLOUR 255 255 255 255
                        DISPLAY_TEXT_WITH_NUMBER 320.0 425.0 G_COST currentClothingPrice
                    endif
                endif
            endif
        endif
    endif

    // Red tracksuit
    if (currentStoreClothingNumber = 13)
    or (currentStoreClothingNumber = 14)
        currentClothingPrice = 50
        
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT11

        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 0.58 1.2
        if (clothes9_created = 0)
            DISPLAY_TEXT 320.0 425.0 STOCK
        else
            if (redTracksuitVariationState = 1)
            and (currentVariation = 1)
                DISPLAY_TEXT 320.0 425.0 CLOTH_3
            else
                if (redTracksuitVariationState = 2)
                and (currentVariation = 2)
                    DISPLAY_TEXT 320.0 425.0 CLOTH_3
                else
                    if (redTracksuitVariationState = 3)
                        DISPLAY_TEXT 320.0 425.0 CLOTH_3
                    else
                        SET_TEXT_COLOUR 255 255 255 255
                        DISPLAY_TEXT_WITH_NUMBER 320.0 425.0 G_COST currentClothingPrice
                    endif
                endif
            endif
        endif
    endif

    USE_TEXT_COMMANDS FALSE

    RETURN

change_clothes:
    // Street
    if (currentStoreClothingNumber = 1)
    or (currentStoreClothingNumber = 2)
        if (not currentClothingModel = 1)
            UNDRESS_CHAR scplayer PLAYER
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 1

        currentVariation = currentStoreClothingNumber
        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation
    endif

    // Soiree
    if (currentStoreClothingNumber = 3)
    or (currentStoreClothingNumber = 4)
        if (not currentClothingModel = 2)
            UNDRESS_CHAR scplayer PLAYER2
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 2

        if (currentStoreClothingNumber = 3)
            currentVariation = 1
        else
            currentVariation = 2
        endif
        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation
    endif

    // Country Club
    if (currentStoreClothingNumber = 5)
    or (currentStoreClothingNumber = 6)
        if not currentClothingModel = 3
            UNDRESS_CHAR scplayer PLAYER4
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 3

        if (currentStoreClothingNumber = 5)
            currentVariation = 1
        else
            currentVariation = 2
        endif
        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation
    endif

    // Casual
    if (currentStoreClothingNumber = 7)
    or (currentStoreClothingNumber = 8)
        if (not currentClothingModel = 4)
            UNDRESS_CHAR scplayer PLAYER8
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 4

        if (currentStoreClothingNumber = 7)
            currentVariation = 1
        else
            currentVariation = 2
        endif
        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation
    endif

    // Mr Vercetti
    if (currentStoreClothingNumber = 9)
    or (currentStoreClothingNumber = 10)
        if not currentClothingModel = 5
            UNDRESS_CHAR scplayer PLAYER9
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 5

        if (currentStoreClothingNumber = 9)
            currentVariation = 1
        else
            currentVariation = 2
        endif
        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation
    endif

    // Black tracksuit
    if (currentStoreClothingNumber = 11)
    or (currentStoreClothingNumber = 12)
        if not currentClothingModel = 6
            UNDRESS_CHAR scplayer PLAY10
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 6

        if (currentStoreClothingNumber = 11)
            currentVariation = 1
        else
            currentVariation = 2
        endif
        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation
    endif

    // Red tracksuit
    if (currentStoreClothingNumber = 13)
    or (currentStoreClothingNumber = 14)
        if not currentClothingModel = 7
            UNDRESS_CHAR scplayer PLAY11
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentClothingModel = 7

        if (currentStoreClothingNumber = 13)
            currentVariation = 1
        else
            currentVariation = 2
        endif
        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation
    endif

    TIMERA = 0
    while TIMERA < 300
        WAIT 0
        GOSUB display_info
    endwhile

    GOTO shop_clothes_loop

buy_clothes:
    if (storedClothingModel = currentClothingModel)
    and (currentStoreClothingNumber = storedStoreClothingNumber)
    and (storedVariation > 0)
        RETURN
    endif

    if (clothes2_created = 0)
    and (currentClothingModel = 2)
        RETURN
    endif
    if (clothes4_created = 0)
    and (currentClothingModel = 3)
        RETURN
    endif
    if (clothes7_created = 0)
    and (currentClothingModel = 4)
        RETURN
    endif
    if (clothes8_created = 0)
    and (currentClothingModel = 5)
        RETURN
    endif
    if (clothes9_created = 0)
    and (currentClothingModel = 6)
        RETURN
    endif
    if (clothes9_created = 0)
    and (currentClothingModel = 7)
        RETURN
    endif

    GET_PLAYER_COORDINATES player1 playera_x playera_y playera_z

    currentClothingPrice--
    if (IS_SCORE_GREATER player1 currentClothingPrice)
	    ADD_ONE_OFF_SOUND playera_x playera_y playera_z SOUND_AMMUNATION_BUY_WEAPON

        currentClothingPrice++
        ADD_MONEY_SPENT_ON_CLOTHES currentClothingPrice
        currentClothingPrice *= -1
        ADD_SCORE player1 currentClothingPrice

        GOSUB change_clothing_state

        while IS_BUTTON_PRESSED PAD1 TRIANGLE // VEHICLE_ENTER_EXIT
            WAIT 0
        endwhile
    else
        PRINT_NOW CLOTH_2 4000 1

	    ADD_ONE_OFF_SOUND playera_x playera_y playera_z SOUND_AMMUNATION_BUY_WEAPON_DENIED

        while IS_BUTTON_PRESSED PAD1 TRIANGLE // VEHICLE_ENTER_EXIT
            WAIT 0
        endwhile

        RETURN
    endif

    storedStoreClothingNumber = currentStoreClothingNumber
    storedClothingModel = currentClothingModel
    storedVariation = currentVariation

    RETURN

change_clothing_state:
    // Street
    if (currentClothingModel = 1)
        if (streetVariationState = 0)
            streetVariationState = currentVariation
            RETURN
        endif
        if (streetVariationState = 1)
            if (currentVariation = 2)
                streetVariationState = 3
            endif
            RETURN
        endif
        if (streetVariationState = 2)
            if (currentVariation = 1)
                streetVariationState = 3
            endif
            RETURN
        endif
    endif

    // Soiree
    if (currentClothingModel = 2)
        if (soireeVariationState = 0)
            soireeVariationState = currentVariation
            RETURN
        endif
        if (soireeVariationState = 1)
            if (currentVariation = 2)
                soireeVariationState = 3
            endif
            RETURN
        endif
        if (soireeVariationState = 2)
            if (currentVariation = 1)
                soireeVariationState = 3
            endif
            RETURN
        endif
    endif

    // Country Club
    if (currentClothingModel = 3)
        if (countryVariationState = 0)
            countryVariationState = currentVariation
            RETURN
        endif
        if (countryVariationState = 1)
            if (currentVariation = 2)
                countryVariationState = 3
            endif
            RETURN
        endif
        if (countryVariationState = 2)
            if (currentVariation = 1)
                countryVariationState = 3
            endif
            RETURN
        endif
    endif

    // Casual
    if (currentClothingModel = 4)
        if (casualVariationState = 0)
            casualVariationState = currentVariation
            RETURN
        endif
        if (casualVariationState = 1)
            if (currentVariation = 2)
                casualVariationState = 3
            endif
            RETURN
        endif
        if (casualVariationState = 2)
            if (currentVariation = 1)
                casualVariationState = 3
            endif
            RETURN
        endif
    endif

    // Mr Vercetti
    if (currentClothingModel = 5)
        if (vercettiVariationState = 0)
            vercettiVariationState = currentVariation
            RETURN
        endif
        if (vercettiVariationState = 1)
            if (currentVariation = 2)
                vercettiVariationState = 3
            endif
            RETURN
        endif
        if (vercettiVariationState = 2)
            if (currentVariation = 1)
                vercettiVariationState = 3
            endif
            RETURN
        endif
    endif

    // Black tracksuit
    if (currentClothingModel = 6)
        if (blackTracksuitVariationState = 0)
            blackTracksuitVariationState = currentVariation
            RETURN
        endif
        if (blackTracksuitVariationState = 1)
            if (currentVariation = 2)
                blackTracksuitVariationState = 3
            endif
            RETURN
        endif
        if blackTracksuitVariationState = 2
            if (currentVariation = 1)
                blackTracksuitVariationState = 3
            endif
            RETURN
        endif
    endif

    // Red tracksuit
    if (currentClothingModel = 7)
        if (redTracksuitVariationState = 0)
            redTracksuitVariationState = currentVariation
            RETURN
        endif
        if (redTracksuitVariationState = 1)
            if (currentVariation = 2)
                redTracksuitVariationState = 3
            endif
            RETURN
        endif
        if redTracksuitVariationState = 2
            if (currentVariation = 1)
                redTracksuitVariationState = 3
            endif
            RETURN
        endif
    endif

    RETURN

leave_store:
    if (storedClothingModel = 1)
        UNDRESS_CHAR scplayer PLAYER
    endif
    if (storedClothingModel = 2)
        UNDRESS_CHAR scplayer PLAYER2
    endif
    if (storedClothingModel = 3)
        UNDRESS_CHAR scplayer PLAYER4
    endif
    if (storedClothingModel = 4)
        UNDRESS_CHAR scplayer PLAYER8
    endif
    if (storedClothingModel = 5)
        UNDRESS_CHAR scplayer PLAYER9
    endif
    if (storedClothingModel = 6)
        UNDRESS_CHAR scplayer PLAY10
    endif
    if (storedClothingModel = 7)
        UNDRESS_CHAR scplayer PLAY11
    endif
    LOAD_ALL_MODELS_NOW
    DRESS_CHAR scplayer

    SET_CHAR_CLOTHING_VARIATION scplayer storedVariation

    RESTORE_CAMERA_JUMPCUT
    SET_CAMERA_BEHIND_PLAYER

    CLEAR_HELP

    SET_PLAYER_CONTROL 0 TRUE
    bPlayerInStore = FALSE

    GOTO shop_clothes_loop
}
