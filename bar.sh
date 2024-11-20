#!/bin/dash

# ^c$var^ = fg color
# ^b$var^ = bg color

interval=0

# load colors
# . ~/.config/chadwm/scripts/bar_themes/onedark
black=#1E1D2D
green=#ABE9B3
white=#D9E0EE
grey=#282737
blue=#96CDFB
red=#F28FAD
darkblue=#83bae8

cpu() {
  cpu_val=$(grep -o "^[^ ]*" /proc/loadavg)

  printf "CPU"
  printf "$cpu_val"
}

pkg_updates() {
  #updates=$({ timeout 20 doas xbps-install -un 2>/dev/null || true; } | wc -l) # void
  updates=$({ timeout 20 checkupdates 2>/dev/null || true; } | wc -l) # arch
  # updates=$({ timeout 20 aptitude search '~U' 2>/dev/null || true; } | wc -l)  # apt (ubuntu, debian etc)

  if [ -z "$updates" ]; then
    printf "      Fully Updated"
  else
    printf "      $updates"" updates"
  fi
}

battery() {
  get_capacity="$(cat /sys/class/power_supply/BAT0/capacity)"
  printf "   $get_capacity"
}

brightness() {
  printf "   "
  printf "%.0f\n" $(cat /sys/class/backlight/*/brightness)
}

mem() {
  printf "  "
  printf " $(free -h | awk '/^Mem/ { print $3 }' | sed s/i//g)"
}

wlan() {
	case "$(cat /sys/class/net/wl*/operstate 2>/dev/null)" in
	up) printf " 󰤨 ^d^%s" " ^c$blue^Connected" ;;
	down) printf "  󰤭 ^d^%s" " Disconnected" ;;
	esac
}

clock() {
	printf "  󱑆 "
	printf " $(date '+%H:%M')  "
}

while true; do

  [ $interval = 0 ] || [ $(($interval % 3600)) = 0 ] && updates=$(pkg_updates)
  interval=$((interval + 1))

  sleep 1 && xsetroot -name "$updates $(battery) $(brightness) $(cpu) $(mem) $(wlan) $(clock)"
done
