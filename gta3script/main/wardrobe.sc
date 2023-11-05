// Vice Extended (ViceEx)
// *****************************************************************************************
// ****************************************Wardrobe*****************************************
// *****************************************************************************************

MISSION_START

// if hotel_clothes_created = 1

SCRIPT_NAME wardrob

SET_DEATHARREST_STATE OFF

mission_wardrobe_start:
	START_NEW_SCRIPT wardrobe_start

	MISSION_END

wardrobe_start:

SCRIPT_NAME WRDRB

{

LVAR_FLOAT wardrobePosX, wardrobePosY, wardrobePosZ, wardrobeHeading
LVAR_INT currentPickupInUse
LVAR_INT storedVariation, currentVariation
LVAR_INT storedWardrobeClothingNumber, currentWardrobeClothingNumber, maxWardrobeClothingNumber

wardrobePosX = 0.0 // 139.0
wardrobePosY = 0.0 // -822.0
wardrobePosZ = 0.0 // 10.0
wardrobeHeading = 0.0
currentPickupInUse = 0
storedVariation = 0
currentVariation = 0
storedWardrobeClothingNumber = 0
currentWardrobeClothingNumber = 1
maxWardrobeClothingNumber = 18

/*CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup4 // ViceEx debug
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup8
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup2
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup11
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup7
clothes4_created = 1
clothes8_created = 1
clothes2_created = 1
clothes9_created = 1
clothes7_created = 1*/

wardrobe_loop:
    WAIT 0 

    if hotel_clothes_created = 1
        if HAS_PICKUP_BEEN_COLLECTED clothes_pickup1
            REMOVE_PICKUP clothes_pickup1

            currentPickupInUse = 1

            wardrobePosX = 228.0
            wardrobePosY = -1266.0
            wardrobePosZ = 20.1
            wardrobeHeading = -100.0

            GOSUB wardrobe_pickup_collected
        endif
    endif

    if mansion_clothes_created = 1
        if HAS_PICKUP_BEEN_COLLECTED mansion_clothes
            REMOVE_PICKUP mansion_clothes

            currentPickupInUse = 2

            wardrobePosX = -381.5
            wardrobePosY = -591.9
            wardrobePosZ = 25.3
            wardrobeHeading = -90.0

            GOSUB wardrobe_pickup_collected
        endif
    endif

    if safehouse_created2 = 1
        if HAS_PICKUP_BEEN_COLLECTED safehouse_clothes2
            REMOVE_PICKUP safehouse_clothes2

            currentPickupInUse = 3

            wardrobePosX = -820.2
            wardrobePosY = 1362.1
            wardrobePosZ = 66.4
            wardrobeHeading = 180.0

            GOSUB wardrobe_pickup_collected
        endif
    endif

    GOTO wardrobe_loop

wardrobe_pickup_collected:
    SET_PLAYER_CONTROL 0 FALSE

    DO_FADE 1000 FADE_OUT
    WAIT 1000

    CLEAR_ALL_CHAR_ANIMS scplayer
    LVAR_FLOAT playerPosY
    playerPosY = wardrobePosZ - 1.5
    SET_CHAR_COORDINATES scplayer, wardrobePosX, wardrobePosY, playerPosY
    SET_CHAR_HEADING scplayer, wardrobeHeading

    LVAR_FLOAT camPosX, camPosY, camPosZ
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS (scplayer, 0.0, 2.5, 1.0, camPosX, camPosY, camPosZ)
    SET_FIXED_CAMERA_POSITION (camPosX, camPosY, camPosZ, 0.0, 0.0, 0.0)
    POINT_CAMERA_AT_CHAR (scplayer 15 2)

    GET_CHAR_CLOTHING_VARIATION scplayer storedVariation
    currentVariation = storedVariation

    /*if storedVariation = 0
        if IS_PLAYER_WEARING player1 player
            UNDRESS_CHAR scplayer play13
            LOAD_ALL_MODELS_NOW
            DRESS_CHAR scplayer
        endif

        currentVariation = 1
        SET_CHAR_CLOTHING_VARIATION scplayer 1
    endif*/

    /*storedClothingModel = 1
    currentClothingModel = storedClothingModel
    storedWardrobeClothingNumber = 1
    currentWardrobeClothingNumber = storedWardrobeClothingNumber*/

    if IS_PLAYER_WEARING player1 player
    or IS_PLAYER_WEARING player1 play13
        if storedVariation = 0
            storedWardrobeClothingNumber = 1
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 7
            else
                storedWardrobeClothingNumber = 13
            endif
        endif
    endif
    if IS_PLAYER_WEARING player1 player2
        if storedVariation = 0
            storedWardrobeClothingNumber = 2
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 8
            else
                storedWardrobeClothingNumber = 14
            endif
        endif
    endif
    if IS_PLAYER_WEARING player1 player4
        if storedVariation = 0
            storedWardrobeClothingNumber = 3
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 9
            else
                storedWardrobeClothingNumber = 15
            endif
        endif
    endif
    if IS_PLAYER_WEARING player1 player8
        if storedVariation = 0
            storedWardrobeClothingNumber = 4
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 10
            else
                storedWardrobeClothingNumber = 16
            endif
        endif
    endif
    if IS_PLAYER_WEARING player1 player9
        if storedVariation = 0
            storedWardrobeClothingNumber = 5
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 11
            else
                storedWardrobeClothingNumber = 17
            endif
        endif
    endif
    if IS_PLAYER_WEARING player1 play10
        if storedVariation = 0
            storedWardrobeClothingNumber = 6
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 12
            else
                storedWardrobeClothingNumber = 18
            endif
        endif
    endif

    currentWardrobeClothingNumber = storedWardrobeClothingNumber

    DO_FADE 1000 FADE_IN
    WAIT 1000

wardrobe_player_loop:
    WAIT 0

    GOSUB wardrobe_display_info

    // Switching clothing variations
    LVAR_INT leftStick_X, unused
    GET_POSITION_OF_ANALOGUE_STICKS 0, leftStick_X, unused, unused, unused
    if (leftStick_X < -64)
        GOSUB wardrobe_before_change_clothes_prev
        GOSUB wardrobe_change_clothes
    else
        if (leftStick_X > 64)
            GOSUB wardrobe_before_change_clothes_next
            GOSUB wardrobe_change_clothes
        endif
    endif

    // Put on clothes
    if (IS_BUTTON_PRESSED PAD1 CROSS)
        GOSUB wardrobe_put_on_clothes
    endif

    // Leave wardrobe
    if not IS_BUTTON_PRESSED PAD1 TRIANGLE)
        GOTO wardrobe_player_loop
    endif

    while (IS_BUTTON_PRESSED PAD1 TRIANGLE)
        WAIT 0
    endwhile

    GOTO leave_wardrobe

wardrobe_display_info:
    PRINT_HELP_FOREVER CLOTH_4

    USE_TEXT_COMMANDS TRUE

    // Displaying clothing name

    // Street
    if (currentWardrobeClothingNumber = 1)
    or (currentWardrobeClothingNumber = 7)
    or (currentWardrobeClothingNumber = 13)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT1
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 2)
    or (currentWardrobeClothingNumber = 8)
    or (currentWardrobeClothingNumber = 14)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT2
    endif

    // Country Club
    if (currentWardrobeClothingNumber = 3)
    or (currentWardrobeClothingNumber = 9)
    or (currentWardrobeClothingNumber = 15)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT4
    endif
    // Casual
    if (currentWardrobeClothingNumber = 4)
    or (currentWardrobeClothingNumber = 10)
    or (currentWardrobeClothingNumber = 16)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT8
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 5)
    or (currentWardrobeClothingNumber = 11)
    or (currentWardrobeClothingNumber = 17)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT9
    endif

    // Tracksuit
    if (currentWardrobeClothingNumber = 6)
    or (currentWardrobeClothingNumber = 12)
    or (currentWardrobeClothingNumber = 18)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT10
    endif

    if (currentWardrobeClothingNumber = storedWardrobeClothingNumber)
        DISPLAY_TEXT 320.0 425.0 CLOTH_5
    endif

    USE_TEXT_COMMANDS FALSE

    RETURN

wardrobe_before_change_clothes_prev:
    if (currentWardrobeClothingNumber = 1)
        currentWardrobeClothingNumber = 19

        if (tracksuitVariationState = 2)
        or (tracksuitVariationState = 3)
            currentWardrobeClothingNumber = 18
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 18)
        if (vercettiVariationState = 2)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 17
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 17)
        if (casualVariationState = 2)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 16
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 16)
        if (countryVariationState = 2)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 15
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 15)
        if (soireeVariationState = 2)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 14
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 14)
        if (streetVariationState = 2)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 13
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 13)
        if (tracksuitVariationState = 1)
        or (tracksuitVariationState = 3)
            currentWardrobeClothingNumber = 12
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 12)
        if (vercettiVariationState = 1)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 11
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 11)
        if (casualVariationState = 1)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 10
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 10)
        if (countryVariationState = 1)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 9
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 9)
        if (soireeVariationState = 1)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 8
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 8)
        if (streetVariationState = 1)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 7
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 7)
    and (clothes9_created = 1)
        currentWardrobeClothingNumber = 6
        return
    endif
    if (currentWardrobeClothingNumber >= 6)
    and (clothes8_created = 1)
        currentWardrobeClothingNumber = 5
        return
    endif
    if (currentWardrobeClothingNumber >= 5)
    and (clothes7_created = 1)
        currentWardrobeClothingNumber = 4
        return
    endif
    if (currentWardrobeClothingNumber >= 4)
    and (clothes4_created = 1)
        currentWardrobeClothingNumber = 3
        return
    endif
    if (currentWardrobeClothingNumber >= 3)
    and (clothes2_created = 1)
        currentWardrobeClothingNumber = 2
        return
    endif

    currentWardrobeClothingNumber = 1
    return

wardrobe_before_change_clothes_next:
    if (currentWardrobeClothingNumber = 1)
    and (clothes2_created = 1)
        currentWardrobeClothingNumber = 2
        return
    endif
    if (currentWardrobeClothingNumber <= 2)
    and (clothes4_created = 1)
        currentWardrobeClothingNumber = 3
        return
    endif
    if (currentWardrobeClothingNumber <= 3)
    and (clothes7_created = 1)
        currentWardrobeClothingNumber = 4
        return
    endif
    if (currentWardrobeClothingNumber <= 4)
    and (clothes8_created = 1)
        currentWardrobeClothingNumber = 5
        return
    endif
    if (currentWardrobeClothingNumber <= 5)
    and (clothes9_created = 1)
        currentWardrobeClothingNumber = 6
        return
    endif
    if (currentWardrobeClothingNumber <= 6)
        if (streetVariationState = 1)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 7
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 7)
        if (soireeVariationState = 1)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 8
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 8)
        if (countryVariationState = 1)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 9
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 9)
        if (casualVariationState = 1)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 10
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 10)
        if (vercettiVariationState = 1)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 11
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 11)
        if (tracksuitVariationState = 1)
        or (tracksuitVariationState = 3)
            currentWardrobeClothingNumber = 12
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 12)
        if (streetVariationState = 2)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 13
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 13)
        if (soireeVariationState = 2)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 14
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 14)
        if (countryVariationState = 2)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 15
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 15)
        if (casualVariationState = 2)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 16
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 16)
        if (vercettiVariationState = 2)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 17
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 17)
        if (tracksuitVariationState = 2)
        or (tracksuitVariationState = 3)
            currentWardrobeClothingNumber = 18
            return
        endif
    endif

    currentWardrobeClothingNumber = 1
    return

wardrobe_change_clothes:
    currentVariation = 0

    // Street
    if (currentWardrobeClothingNumber = 1)
        UNDRESS_CHAR scplayer player
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 2)
        UNDRESS_CHAR scplayer PLAYER2
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Country Club
    if (currentWardrobeClothingNumber = 3)
        UNDRESS_CHAR scplayer PLAYER4
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Casual
    if (currentWardrobeClothingNumber = 4)
        UNDRESS_CHAR scplayer PLAYER8
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 5)
        UNDRESS_CHAR scplayer PLAYER9
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Tracksuit
    if (currentWardrobeClothingNumber = 6)
        UNDRESS_CHAR scplayer PLAY10
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    currentVariation = 1

    // Street
    if (currentWardrobeClothingNumber = 7)
        UNDRESS_CHAR scplayer PLAY13
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 8)
        UNDRESS_CHAR scplayer PLAYER2
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif


    // Country Club
    if (currentWardrobeClothingNumber = 9)
        UNDRESS_CHAR scplayer PLAYER4
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Casual
    if (currentWardrobeClothingNumber = 10)
        UNDRESS_CHAR scplayer PLAYER8
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 11)
        UNDRESS_CHAR scplayer PLAYER9
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Tracksuit
    if (currentWardrobeClothingNumber = 12)
        UNDRESS_CHAR scplayer PLAY10
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    currentVariation = 2

    // Street
    if (currentWardrobeClothingNumber = 13)
        UNDRESS_CHAR scplayer PLAY13
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 14)
        UNDRESS_CHAR scplayer PLAYER2
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif


    // Country Club
    if (currentWardrobeClothingNumber = 15)
        UNDRESS_CHAR scplayer PLAYER4
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Casual
    if (currentWardrobeClothingNumber = 16)
        UNDRESS_CHAR scplayer PLAYER8
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 17)
        UNDRESS_CHAR scplayer PLAYER9
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Tracksuit
    if (currentWardrobeClothingNumber = 18)
        UNDRESS_CHAR scplayer PLAY10
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

wardrobe_put_on_clothes:
    //if (storedClothingModel = currentClothingModel)
    if (currentWardrobeClothingNumber = storedWardrobeClothingNumber)
        RETURN
    endif

    while (IS_BUTTON_PRESSED PAD1 TRIANGLE) // VEHICLE_ENTER_EXIT
        WAIT 0
    endwhile

    GET_PLAYER_COORDINATES player1 playera_x playera_y playera_z
    ADD_ONE_OFF_SOUND playera_x playera_y playera_z SOUND_AMMUNATION_BUY_WEAPON

    storedWardrobeClothingNumber = currentWardrobeClothingNumber
    //storedClothingModel = currentClothingModel
    storedVariation = currentVariation

    RETURN

leave_wardrobe:
    if (storedWardrobeClothingNumber = 1)
        UNDRESS_CHAR scplayer player
    endif
    if (storedWardrobeClothingNumber = 7)
    or (storedWardrobeClothingNumber = 13)
        UNDRESS_CHAR scplayer PLAY13
    endif
    if (storedWardrobeClothingNumber = 2)
    or (storedWardrobeClothingNumber = 8)
    or (storedWardrobeClothingNumber = 14)
        UNDRESS_CHAR scplayer PLAYER2
    endif
    if (storedWardrobeClothingNumber = 3)
    or (storedWardrobeClothingNumber = 9)
    or (storedWardrobeClothingNumber = 15)
        UNDRESS_CHAR scplayer PLAYER4
    endif
    if (storedWardrobeClothingNumber = 4)
    or (storedWardrobeClothingNumber = 10)
    or (storedWardrobeClothingNumber = 16)
        UNDRESS_CHAR scplayer PLAYER8
    endif
    if (storedWardrobeClothingNumber = 5)
    or (storedWardrobeClothingNumber = 11)
    or (storedWardrobeClothingNumber = 17)
        UNDRESS_CHAR scplayer PLAYER9
    endif
    if (storedWardrobeClothingNumber = 6)
    or (storedWardrobeClothingNumber = 12)
    or (storedWardrobeClothingNumber = 18)
        UNDRESS_CHAR scplayer PLAY10
    endif
    LOAD_ALL_MODELS_NOW
    DRESS_CHAR scplayer

    SET_CHAR_CLOTHING_VARIATION scplayer storedVariation

    RESTORE_CAMERA_JUMPCUT
    SET_CAMERA_BEHIND_PLAYER

    CLEAR_HELP

    SET_PLAYER_CONTROL 0 TRUE

    if (currentPickupInUse = 1)
        CREATE_CLOTHES_PICKUP 226.4 -1265.6 20.1 1 clothes_pickup1
    endif
    if (currentPickupInUse = 2)
        CREATE_CLOTHES_PICKUP -384.5 -591.9 25.3 1 mansion_clothes
    endif
    if (currentPickupInUse = 3)
        CREATE_CLOTHES_PICKUP -820.2 1364.1 66.4 1 safehouse_clothes2
    endif
    currentPickupInUse = 0

    wardrobePosX = 0.0
    wardrobePosY = 0.0
    wardrobePosZ = 0.0
    wardrobeHeading = 0.0
    currentPickupInUse = 0
    storedVariation = 0
    currentVariation = 0
    storedWardrobeClothingNumber = 0
    currentWardrobeClothingNumber = 1
    maxWardrobeClothingNumber = 18

    GOTO wardrobe_loop

wardrobe_wait:
    TIMERA = 0
    while (TIMERA < 300)
        WAIT 0
        GOSUB wardrobe_display_info
    endwhile

    RETURN
}