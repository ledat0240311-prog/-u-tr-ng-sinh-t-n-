package com.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.DirectionsRun
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Security
import androidx.compose.material.icons.filled.ShoppingBag
import androidx.compose.material.icons.filled.Speed
import androidx.compose.material.icons.filled.TrackChanges
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import com.example.audio.GameSoundManager
import com.example.ui.GameGLSurfaceView
import com.example.ui.theme.MyApplicationTheme
import kotlinx.coroutines.delay
import kotlin.math.roundToInt
import kotlin.math.sqrt

class MainActivity : ComponentActivity() {

    private lateinit var soundManager: GameSoundManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        soundManager = GameSoundManager(this)

        setContent {
            MyApplicationTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = Color.Black
                ) {
                    SurvivalGameScreen(soundManager = soundManager)
                }
            }
        }
    }
}

data class HUDState(
    val hp: Float = 100f,
    val maxHp: Float = 100f,
    val armor: Float = 50f,
    val maxArmor: Float = 50f,
    val stamina: Float = 100f,
    val maxStamina: Float = 100f,
    val currentAmmo: Int = 30,
    val reserveAmmo: Int = 120,
    val weaponSlot: Int = 0,
    val weaponName: String = "9mm Tactical",
    val waveNumber: Int = 1,
    val enemiesRemaining: Int = 5,
    val totalKills: Int = 0,
    val score: Int = 0,
    val coins: Int = 100,
    val waveState: Int = 0, // 0: prep, 1: in_progress, 2: cleared, 3: game_over
    val stateTimer: Float = 0f,
    val hitmarkerActive: Boolean = false,
    val damageVignette: Float = 0f
)

@Composable
fun SurvivalGameScreen(soundManager: GameSoundManager) {
    var hud by remember { mutableStateOf(HUDState()) }
    var isAiming by remember { mutableStateOf(false) }
    var isFiring by remember { mutableStateOf(false) }
    var isSprinting by remember { mutableStateOf(false) }
    var showUpgradeDialog by remember { mutableStateOf(false) }

    val rawFloats = remember { FloatArray(18) }
    val stringHolder = remember { Array(1) { "9mm Tactical" } }
    val audioBuffer = remember { IntArray(32) }

    // Audio & HUD data update loop (approx 30Hz)
    LaunchedEffect(Unit) {
        while (true) {
            try {
                NativeBridge.nativeGetHUDData(rawFloats, stringHolder)
                hud = HUDState(
                    hp = rawFloats[0],
                    maxHp = rawFloats[1],
                    armor = rawFloats[2],
                    maxArmor = rawFloats[3],
                    stamina = rawFloats[4],
                    maxStamina = rawFloats[5],
                    currentAmmo = rawFloats[6].toInt(),
                    reserveAmmo = rawFloats[7].toInt(),
                    weaponSlot = rawFloats[8].toInt(),
                    weaponName = stringHolder[0],
                    waveNumber = rawFloats[9].toInt(),
                    enemiesRemaining = rawFloats[10].toInt(),
                    totalKills = rawFloats[11].toInt(),
                    score = rawFloats[12].toInt(),
                    coins = rawFloats[13].toInt(),
                    waveState = rawFloats[14].toInt(),
                    stateTimer = rawFloats[15],
                    hitmarkerActive = rawFloats[16] > 0.5f,
                    damageVignette = rawFloats[17]
                )

                // Drain audio events and play synthesized sfx
                val count = NativeBridge.nativeDrainAudioEvents(audioBuffer)
                for (i in 0 until count) {
                    soundManager.playEvent(audioBuffer[i])
                }
            } catch (_: Exception) {
            }
            delay(32)
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        // 1. OpenGL ES 3.0 Surface View
        AndroidView(
            factory = { ctx ->
                GameGLSurfaceView(ctx) { _, _ -> }
            },
            modifier = Modifier.fillMaxSize()
        )

        // 2. Damage Vignette Flash
        if (hud.damageVignette > 0.05f) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(
                        Brush.radialGradient(
                            colors = listOf(
                                Color.Transparent,
                                Color.Red.copy(alpha = (hud.damageVignette * 0.45f).coerceIn(0f, 0.7f))
                            )
                        )
                    )
            )
        }

        // 3. Center Crosshair & Hitmarker
        CenterCrosshair(isAiming = isAiming, isHit = hud.hitmarkerActive)

        // 4. Top Header HUD (Wave, Enemies, Score, Coins)
        TopHUDBar(hud = hud)

        // 5. Left Status Bars (HP, Armor, Stamina)
        LeftStatusHUD(
            hud = hud,
            modifier = Modifier
                .align(Alignment.TopStart)
                .padding(start = 16.dp, top = 64.dp)
        )

        // 6. Left Virtual Movement Joystick
        VirtualJoystick(
            isSprinting = isSprinting,
            onSprintToggle = {
                isSprinting = !isSprinting
            },
            onMove = { x, y ->
                NativeBridge.nativeOnTouchMove(x, y, isSprinting)
            },
            modifier = Modifier
                .align(Alignment.BottomStart)
                .padding(start = 24.dp, bottom = 24.dp)
        )

        // 7. Weapon Quick-Select Bar (Bottom Center)
        WeaponBar(
            currentSlot = hud.weaponSlot,
            onSelectSlot = { slot ->
                NativeBridge.nativeOnSwitchWeapon(slot)
            },
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .padding(bottom = 16.dp)
        )

        // 8. Right Tactical Combat Action Controls
        RightCombatControls(
            ammoCurrent = hud.currentAmmo,
            ammoReserve = hud.reserveAmmo,
            weaponName = hud.weaponName,
            isAiming = isAiming,
            isFiring = isFiring,
            onFireChange = { pressed ->
                isFiring = pressed
                NativeBridge.nativeOnFire(pressed)
            },
            onAimToggle = {
                isAiming = !isAiming
                NativeBridge.nativeOnAim(isAiming)
            },
            onReload = {
                NativeBridge.nativeOnReload()
            },
            onJump = {
                NativeBridge.nativeOnJump()
            },
            onCrouch = {
                NativeBridge.nativeOnCrouch()
            },
            onOpenUpgrades = {
                showUpgradeDialog = true
            },
            modifier = Modifier
                .align(Alignment.BottomEnd)
                .padding(end = 24.dp, bottom = 20.dp)
        )

        // 9. Wave Cleared Banner
        if (hud.waveState == 2) {
            Box(
                modifier = Modifier
                    .align(Alignment.Center)
                    .background(Color(0xCC111827), RoundedCornerShape(12.dp))
                    .border(2.dp, Color(0xFF10B981), RoundedCornerShape(12.dp))
                    .padding(horizontal = 28.dp, vertical = 16.dp)
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = "WAVE CLEARED!",
                        color = Color(0xFF10B981),
                        fontSize = 28.sp,
                        fontWeight = FontWeight.Black
                    )
                    Text(
                        text = "Bonus: +${hud.waveNumber * 50} pts  •  +${50 + hud.waveNumber * 20} coins",
                        color = Color.White,
                        fontSize = 14.sp
                    )
                }
            }
        }

        // 10. Upgrade Shop Dialog
        if (showUpgradeDialog || hud.waveState == 2 && hud.stateTimer > 2.0f) {
            UpgradeShopModal(
                coins = hud.coins,
                onClose = { showUpgradeDialog = false },
                onUpgrade = { type ->
                    NativeBridge.nativeApplyUpgrade(type)
                }
            )
        }

        // 11. Game Over Screen
        if (hud.waveState == 3) {
            GameOverModal(
                wave = hud.waveNumber,
                score = hud.score,
                kills = hud.totalKills,
                onRestart = {
                    NativeBridge.nativeRestart()
                }
            )
        }
    }
}

@Composable
fun TopHUDBar(hud: HUDState) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(top = 10.dp, start = 16.dp, end = 16.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Wave Badge
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier
                .background(Color(0xBB1E293B), RoundedCornerShape(8.dp))
                .border(1.dp, Color(0xFFE2E8F0).copy(alpha = 0.3f), RoundedCornerShape(8.dp))
                .padding(horizontal = 12.dp, vertical = 6.dp)
        ) {
            Text(
                text = "WAVE ${String.format("%02d", hud.waveNumber)}",
                color = Color(0xFFF97316),
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold
            )
            Spacer(modifier = Modifier.width(12.dp))
            Text(
                text = "ENEMIES: ${hud.enemiesRemaining}",
                color = Color.White,
                fontSize = 14.sp,
                fontWeight = FontWeight.SemiBold
            )
        }

        // Score & Coins
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier
                .background(Color(0xBB1E293B), RoundedCornerShape(8.dp))
                .border(1.dp, Color(0xFFE2E8F0).copy(alpha = 0.3f), RoundedCornerShape(8.dp))
                .padding(horizontal = 12.dp, vertical = 6.dp)
        ) {
            Text(
                text = "SCORE: ${hud.score}",
                color = Color(0xFF38BDF8),
                fontSize = 14.sp,
                fontWeight = FontWeight.Bold
            )
            Spacer(modifier = Modifier.width(14.dp))
            Text(
                text = "🪙 ${hud.coins}",
                color = Color(0xFFFACC15),
                fontSize = 14.sp,
                fontWeight = FontWeight.Bold
            )
        }
    }
}

@Composable
fun LeftStatusHUD(hud: HUDState, modifier: Modifier = Modifier) {
    Column(
        modifier = modifier
            .background(Color(0xAA0F172A), RoundedCornerShape(8.dp))
            .border(1.dp, Color(0xFF334155), RoundedCornerShape(8.dp))
            .padding(10.dp)
            .width(160.dp)
    ) {
        // HP Bar
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text("HP", color = Color(0xFFEF4444), fontSize = 11.sp, fontWeight = FontWeight.Bold)
            Text("${hud.hp.toInt()}/${hud.maxHp.toInt()}", color = Color.White, fontSize = 11.sp)
        }
        StatBar(
            current = hud.hp,
            max = hud.maxHp,
            color = Color(0xFFEF4444),
            modifier = Modifier.padding(top = 2.dp, bottom = 6.dp)
        )

        // Armor Bar
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text("ARMOR", color = Color(0xFF3B82F6), fontSize = 11.sp, fontWeight = FontWeight.Bold)
            Text("${hud.armor.toInt()}/${hud.maxArmor.toInt()}", color = Color.White, fontSize = 11.sp)
        }
        StatBar(
            current = hud.armor,
            max = hud.maxArmor,
            color = Color(0xFF3B82F6),
            modifier = Modifier.padding(top = 2.dp, bottom = 6.dp)
        )

        // Stamina Bar
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text("STAMINA", color = Color(0xFFF59E0B), fontSize = 11.sp, fontWeight = FontWeight.Bold)
            Text("${hud.stamina.toInt()}%", color = Color.White, fontSize = 11.sp)
        }
        StatBar(
            current = hud.stamina,
            max = hud.maxStamina,
            color = Color(0xFFF59E0B),
            modifier = Modifier.padding(top = 2.dp)
        )
    }
}

@Composable
fun StatBar(current: Float, max: Float, color: Color, modifier: Modifier = Modifier) {
    val progress = if (max > 0f) (current / max).coerceIn(0f, 1f) else 0f
    Box(
        modifier = modifier
            .fillMaxWidth()
            .height(7.dp)
            .clip(RoundedCornerShape(4.dp))
            .background(Color(0xFF334155))
    ) {
        Box(
            modifier = Modifier
                .fillMaxWidth(progress)
                .height(7.dp)
                .background(color)
        )
    }
}

@Composable
fun CenterCrosshair(isAiming: Boolean, isHit: Boolean) {
    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        val crosshairSpread = if (isAiming) 8.dp else 16.dp
        val crosshairColor = if (isHit) Color.Red else Color.White.copy(alpha = 0.8f)

        Canvas(modifier = Modifier.size(48.dp)) {
            val center = Offset(size.width / 2f, size.height / 2f)
            val spreadPx = crosshairSpread.toPx()
            val len = 10.dp.toPx()
            val stroke = 2.dp.toPx()

            // Top
            drawLine(crosshairColor, Offset(center.x, center.y - spreadPx - len), Offset(center.x, center.y - spreadPx), stroke)
            // Bottom
            drawLine(crosshairColor, Offset(center.x, center.y + spreadPx), Offset(center.x, center.y + spreadPx + len), stroke)
            // Left
            drawLine(crosshairColor, Offset(center.x - spreadPx - len, center.y), Offset(center.x - spreadPx, center.y), stroke)
            // Right
            drawLine(crosshairColor, Offset(center.x + spreadPx, center.y), Offset(center.x + spreadPx + len, center.y), stroke)

            // Hitmarker X
            if (isHit) {
                val hitSize = 9.dp.toPx()
                val hitColor = Color.Red
                drawLine(hitColor, Offset(center.x - hitSize, center.y - hitSize), Offset(center.x + hitSize, center.y + hitSize), 3.dp.toPx())
                drawLine(hitColor, Offset(center.x - hitSize, center.y + hitSize), Offset(center.x + hitSize, center.y - hitSize), 3.dp.toPx())
            }
        }
    }
}

@Composable
fun VirtualJoystick(
    isSprinting: Boolean,
    onSprintToggle: () -> Unit,
    onMove: (Float, Float) -> Unit,
    modifier: Modifier = Modifier
) {
    var thumbX by remember { mutableFloatStateOf(0f) }
    var thumbY by remember { mutableFloatStateOf(0f) }
    val maxRadius = 55f

    Column(
        modifier = modifier,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        // Sprint Toggle Button
        IconButton(
            onClick = onSprintToggle,
            modifier = Modifier
                .size(44.dp)
                .background(
                    if (isSprinting) Color(0xFFF97316) else Color(0x99334155),
                    CircleShape
                )
                .border(1.dp, Color.White.copy(alpha = 0.4f), CircleShape)
                .testTag("sprint_button")
        ) {
            Icon(
                Icons.AutoMirrored.Filled.DirectionsRun,
                contentDescription = "Sprint",
                tint = Color.White,
                modifier = Modifier.size(24.dp)
            )
        }

        Spacer(modifier = Modifier.height(8.dp))

        // Joystick Base & Thumb
        Box(
            modifier = Modifier
                .size(130.dp)
                .background(Color(0x55000000), CircleShape)
                .border(2.dp, Color(0x66FFFFFF), CircleShape)
                .pointerInput(Unit) {
                    detectDragGestures(
                        onDragEnd = {
                            thumbX = 0f
                            thumbY = 0f
                            onMove(0f, 0f)
                        },
                        onDragCancel = {
                            thumbX = 0f
                            thumbY = 0f
                            onMove(0f, 0f)
                        },
                        onDrag = { change, dragAmount ->
                            change.consume()
                            val nextX = thumbX + dragAmount.x
                            val nextY = thumbY + dragAmount.y
                            val dist = sqrt(nextX * nextX + nextY * nextY)
                            if (dist > maxRadius) {
                                thumbX = (nextX / dist) * maxRadius
                                thumbY = (nextY / dist) * maxRadius
                            } else {
                                thumbX = nextX
                                thumbY = nextY
                            }
                            // Normalize: x in [-1, 1], y in [-1, 1] (y inverted: up is forward)
                            val normX = thumbX / maxRadius
                            val normY = -thumbY / maxRadius
                            onMove(normX, normY)
                        }
                    )
                },
            contentAlignment = Alignment.Center
        ) {
            Box(
                modifier = Modifier
                    .offset { IntOffset(thumbX.roundToInt(), thumbY.roundToInt()) }
                    .size(54.dp)
                    .background(Color(0xCCF97316), CircleShape)
                    .border(2.dp, Color.White, CircleShape)
            )
        }
    }
}

@Composable
fun WeaponBar(
    currentSlot: Int,
    onSelectSlot: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val weapons = listOf("PISTOL", "SMG", "M4A1", "SHOTGUN", "SNIPER")
    Row(
        modifier = modifier
            .background(Color(0xCC0F172A), RoundedCornerShape(12.dp))
            .border(1.dp, Color(0xFF334155), RoundedCornerShape(12.dp))
            .padding(4.dp),
        horizontalArrangement = Arrangement.spacedBy(4.dp)
    ) {
        weapons.forEachIndexed { index, name ->
            val isSelected = currentSlot == index
            Button(
                onClick = { onSelectSlot(index) },
                colors = ButtonDefaults.buttonColors(
                    containerColor = if (isSelected) Color(0xFFF97316) else Color(0x441E293B)
                ),
                shape = RoundedCornerShape(8.dp),
                modifier = Modifier
                    .height(34.dp)
                    .testTag("weapon_slot_$index")
            ) {
                Text(
                    text = name,
                    fontSize = 11.sp,
                    fontWeight = FontWeight.Bold,
                    color = if (isSelected) Color.White else Color(0xFFCBD5E1)
                )
            }
        }
    }
}

@Composable
fun RightCombatControls(
    ammoCurrent: Int,
    ammoReserve: Int,
    weaponName: String,
    isAiming: Boolean,
    isFiring: Boolean,
    onFireChange: (Boolean) -> Unit,
    onAimToggle: () -> Unit,
    onReload: () -> Unit,
    onJump: () -> Unit,
    onCrouch: () -> Unit,
    onOpenUpgrades: () -> Unit,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier,
        horizontalAlignment = Alignment.End
    ) {
        // Ammo & Weapon badge
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier
                .background(Color(0xDD0F172A), RoundedCornerShape(8.dp))
                .border(1.dp, Color(0xFF334155), RoundedCornerShape(8.dp))
                .padding(horizontal = 14.dp, vertical = 6.dp)
        ) {
            Column(horizontalAlignment = Alignment.End) {
                Text(
                    text = weaponName,
                    color = Color(0xFF94A3B8),
                    fontSize = 11.sp,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    text = "$ammoCurrent / $ammoReserve",
                    color = if (ammoCurrent <= 5) Color(0xFFEF4444) else Color.White,
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Black
                )
            }
        }

        Spacer(modifier = Modifier.height(10.dp))

        // Tactical buttons row
        Row(
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            // Upgrade Shop Button
            IconButton(
                onClick = onOpenUpgrades,
                modifier = Modifier
                    .size(46.dp)
                    .background(Color(0xFF0284C7), CircleShape)
                    .border(1.dp, Color.White.copy(alpha = 0.5f), CircleShape)
                    .testTag("upgrade_shop_button")
            ) {
                Icon(Icons.Default.ShoppingBag, contentDescription = "Shop", tint = Color.White)
            }

            // Crouch Button
            IconButton(
                onClick = onCrouch,
                modifier = Modifier
                    .size(46.dp)
                    .background(Color(0x99334155), CircleShape)
                    .border(1.dp, Color.White.copy(alpha = 0.4f), CircleShape)
                    .testTag("crouch_button")
            ) {
                Text("CR", color = Color.White, fontWeight = FontWeight.Bold, fontSize = 13.sp)
            }

            // Jump Button
            IconButton(
                onClick = onJump,
                modifier = Modifier
                    .size(46.dp)
                    .background(Color(0x99334155), CircleShape)
                    .border(1.dp, Color.White.copy(alpha = 0.4f), CircleShape)
                    .testTag("jump_button")
            ) {
                Icon(Icons.Default.KeyboardArrowUp, contentDescription = "Jump", tint = Color.White)
            }

            // Reload Button
            IconButton(
                onClick = onReload,
                modifier = Modifier
                    .size(52.dp)
                    .background(Color(0xFF2563EB), CircleShape)
                    .border(2.dp, Color.White.copy(alpha = 0.6f), CircleShape)
                    .testTag("reload_button")
            ) {
                Icon(Icons.Default.Refresh, contentDescription = "Reload", tint = Color.White)
            }

            // Aim (ADS) Button
            IconButton(
                onClick = onAimToggle,
                modifier = Modifier
                    .size(52.dp)
                    .background(
                        if (isAiming) Color(0xFF10B981) else Color(0x99334155),
                        CircleShape
                    )
                    .border(2.dp, Color.White, CircleShape)
                    .testTag("aim_button")
            ) {
                Icon(Icons.Default.TrackChanges, contentDescription = "Aim", tint = Color.White)
            }

            // Big Fire Button
            Box(
                modifier = Modifier
                    .size(76.dp)
                    .background(
                        if (isFiring) Color(0xFFB91C1C) else Color(0xFFDC2626),
                        CircleShape
                    )
                    .border(3.dp, Color.White, CircleShape)
                    .pointerInput(Unit) {
                        detectTapGestures(
                            onPress = {
                                onFireChange(true)
                                tryAwaitRelease()
                                onFireChange(false)
                            }
                        )
                    }
                    .testTag("fire_button"),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = "FIRE",
                    color = Color.White,
                    fontWeight = FontWeight.Black,
                    fontSize = 17.sp
                )
            }
        }
    }
}

@Composable
fun UpgradeShopModal(
    coins: Int,
    onClose: () -> Unit,
    onUpgrade: (Int) -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color(0xAA000000))
            .pointerInput(Unit) {},
        contentAlignment = Alignment.Center
    ) {
        Card(
            modifier = Modifier
                .width(420.dp)
                .padding(16.dp),
            colors = CardDefaults.cardColors(containerColor = Color(0xFF0F172A)),
            shape = RoundedCornerShape(16.dp)
        ) {
            Column(modifier = Modifier.padding(18.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "TACTICAL ARMORY & UPGRADES",
                        color = Color(0xFFF97316),
                        fontSize = 16.sp,
                        fontWeight = FontWeight.Black
                    )
                    IconButton(onClick = onClose, modifier = Modifier.size(32.dp)) {
                        Icon(Icons.Default.Close, contentDescription = "Close", tint = Color.White)
                    }
                }

                Text(
                    text = "Current Coins: 🪙 $coins",
                    color = Color(0xFFFACC15),
                    fontSize = 14.sp,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier.padding(bottom = 12.dp)
                )

                LazyColumn(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                    item { UpgradeItemRow("Damage +20%", "Increase damage for all guns", 120, coins) { onUpgrade(1) } }
                    item { UpgradeItemRow("Max HP +25", "Increase survivability", 80, coins) { onUpgrade(2) } }
                    item { UpgradeItemRow("Max Armor +25", "Reinforce Kevlar armor plates", 80, coins) { onUpgrade(3) } }
                    item { UpgradeItemRow("Speed +12%", "Run and flank faster", 100, coins) { onUpgrade(4) } }
                    item { UpgradeItemRow("Reload Speed +20%", "Faster magazine changes", 90, coins) { onUpgrade(5) } }
                    item { UpgradeItemRow("Magazine Size +5", "More ammo per clip", 110, coins) { onUpgrade(6) } }
                    item { UpgradeItemRow("Critical Chance +8%", "More deadly headshots & crits", 130, coins) { onUpgrade(7) } }
                }

                Spacer(modifier = Modifier.height(14.dp))
                Button(
                    onClick = onClose,
                    modifier = Modifier.fillMaxWidth(),
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF10B981))
                ) {
                    Text("CONTINUE BATTLE", fontWeight = FontWeight.Bold)
                }
            }
        }
    }
}

@Composable
fun UpgradeItemRow(
    title: String,
    desc: String,
    cost: Int,
    currentCoins: Int,
    onBuy: () -> Unit
) {
    val canAfford = currentCoins >= cost
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF1E293B), RoundedCornerShape(8.dp))
            .padding(10.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(title, color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
            Text(desc, color = Color(0xFF94A3B8), fontSize = 11.sp)
        }
        Button(
            onClick = onBuy,
            enabled = canAfford,
            colors = ButtonDefaults.buttonColors(
                containerColor = Color(0xFFF59E0B),
                disabledContainerColor = Color(0xFF334155)
            ),
            shape = RoundedCornerShape(6.dp),
            modifier = Modifier.height(32.dp)
        ) {
            Text("🪙 $cost", fontSize = 11.sp, fontWeight = FontWeight.Bold)
        }
    }
}

@Composable
fun GameOverModal(
    wave: Int,
    score: Int,
    kills: Int,
    onRestart: () -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color(0xDD000000)),
        contentAlignment = Alignment.Center
    ) {
        Card(
            modifier = Modifier
                .width(360.dp)
                .padding(20.dp),
            colors = CardDefaults.cardColors(containerColor = Color(0xFF18181B)),
            shape = RoundedCornerShape(16.dp),
            border = androidx.compose.foundation.BorderStroke(2.dp, Color(0xFFEF4444))
        ) {
            Column(
                modifier = Modifier.padding(24.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = "YOU DIED",
                    color = Color(0xFFEF4444),
                    fontSize = 32.sp,
                    fontWeight = FontWeight.Black
                )
                Text(
                    text = "SURVIVOR DOWN IN ACTION",
                    color = Color(0xFFA1A1AA),
                    fontSize = 12.sp,
                    letterSpacing = 2.sp,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier.padding(bottom = 16.dp)
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceAround
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Text("WAVE", color = Color(0xFFA1A1AA), fontSize = 11.sp)
                        Text("$wave", color = Color.White, fontSize = 20.sp, fontWeight = FontWeight.Bold)
                    }
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Text("KILLS", color = Color(0xFFA1A1AA), fontSize = 11.sp)
                        Text("$kills", color = Color(0xFFF97316), fontSize = 20.sp, fontWeight = FontWeight.Bold)
                    }
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Text("SCORE", color = Color(0xFFA1A1AA), fontSize = 11.sp)
                        Text("$score", color = Color(0xFF38BDF8), fontSize = 20.sp, fontWeight = FontWeight.Bold)
                    }
                }

                Spacer(modifier = Modifier.height(24.dp))

                Button(
                    onClick = onRestart,
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(48.dp)
                        .testTag("restart_button"),
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFFEF4444)),
                    shape = RoundedCornerShape(8.dp)
                ) {
                    Icon(Icons.Default.Refresh, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("DEPLOY AGAIN", fontWeight = FontWeight.Bold, fontSize = 15.sp)
                }
            }
        }
    }
}
