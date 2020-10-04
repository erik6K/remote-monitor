/*get_telemetry fetches the battery voltage and mains status using
the /telemetry/<device_id> API and dynamically displays this data within
the webpage*/
var get_telemetry = function () {
    $('.device').each(function () {
        let id = $(this).attr('id');
        var url = '/telemetry/' + id;
        $.ajax(url).done(
            function (data) {
                $('#' + id + '_mains').html(data.mains.value);
                $('#' + id + '_battery').html(data.battery.value);
            }
        );
    });
}

setInterval(get_telemetry, 60 * 1000);

$(document).ready(get_telemetry)

