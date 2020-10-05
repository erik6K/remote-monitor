/* Function snipper sourced from https://stackoverflow.com/questions/8211744/convert-time-interval-given-in-seconds-into-more-human-readable-form*/
function millisecondsToStr (milliseconds) {
    // TIP: to find current time in milliseconds, use:
    // var  current_time_milliseconds = new Date().getTime();

    function numberEnding (number) {
        return (number > 1) ? 's' : '';
    }

    var temp = Math.floor(milliseconds / 1000);
    var years = Math.floor(temp / 31536000);
    if (years) {
        return years + ' year' + numberEnding(years);
    }
    //TODO: Months! Maybe weeks? 
    var days = Math.floor((temp %= 31536000) / 86400);
    if (days) {
        return days + ' day' + numberEnding(days);
    }
    var hours = Math.floor((temp %= 86400) / 3600);
    if (hours) {
        return hours + ' hour' + numberEnding(hours);
    }
    var minutes = Math.floor((temp %= 3600) / 60);
    if (minutes) {
        return minutes + ' minute' + numberEnding(minutes);
    }
    var seconds = temp % 60;
    if (seconds) {
        return seconds + ' second' + numberEnding(seconds);
    }
    return 'less than a second'; //'just now' //or other string you like;
}

var update_time = function () {
    $('.time').each(function () {
        var d = new Date($(this).attr("data-time"));
        $(this).html(millisecondsToStr(new Date() - d));
    })
}

/*get_telemetry fetches the battery voltage and mains status using
the /telemetry/<device_id> API and dynamically displays this data within
the webpage*/
var get_telemetry = function () {
    $('.device').each(function () {
        let id = $(this).attr('id');
        var url = '/telemetry/' + id;
        $.ajax(url).done(
            function (data) {
                $('#' + id + '_mains_value').html(data.mains.value);
                (data.mains.value == "ON") ? color = "lime" : color = "red";
                $('#' + id + '_mains_value').css("background-color",color);
                var d = new Date(data.mains.timestamp);
                $('#' + id + '_mains_time').html(millisecondsToStr(new Date() - d));
                $('#' + id + '_mains_time').attr("data-time", data.mains.timestamp);
                $('#' + id + '_battery_value').html(data.battery.value + " V");
                var d = new Date(data.battery.timestamp);
                $('#' + id + '_battery_time').html(millisecondsToStr(new Date() - d));
                $('#' + id + '_battery_time').attr("data-time", data.battery.timestamp);

            }
        );
    });
}

setInterval(get_telemetry, 60 * 1000);

setInterval(update_time, 1 * 1000);

$(document).ready(get_telemetry)

