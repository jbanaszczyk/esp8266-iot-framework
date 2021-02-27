import React, {useEffect, useState} from "react";

import {Button, Form, Spinner, unHidePass} from "./UiComponents";
import {CornerDownRight, Eye, Lock, Server, Wifi} from "react-feather";

import Config from "./../configuration.json";

let loc;
if (Config.find(entry => entry.name === "language")) {
    loc = require("./../lang/" + Config.find(entry => entry.name === "language").value + ".json");
} else {
    loc = require("./../lang/en.json");
}

export function WifiPage(props) {
    const [state, setState] = useState({apMode: true, ssid: "", dhcp: true, localIP: "", subnetMask: "", gatewayIP: "", dnsIP: ""});
    const [dhcpForm, setDhcpForm] = useState(state.dhcp);

    useEffect(() => {
        document.title = loc.titleWifi;
        fetch(`${props.API}/api/wifi/get`)
            .then((response) => {
                return response.json();
            })
            .then((data) => {
                setState(data);
                setDhcpForm(state.dhcp);
            });
    }, []);

    function change_Wifi_settings() {
        if (dhcpForm) {
            fetch(`${props.API}/api/wifi/set?ssid=${escape(document.getElementById("ssid").value.trim())}&pass=${escape(document.getElementById("pass").value.trim())}`,
                {method: "POST"})
                .then();
        } else {
            fetch(`${props.API}/api/wifi/setStatic?ssid=${escape(document.getElementById("ssid").value.trim())}&pass=${escape(document.getElementById("pass").value.trim())}&localIP=${escape(document.getElementById("localIP").value.trim())}&subnetMask=${escape(document.getElementById("subnetMask").value.trim())}&gatewayIP=${escape(document.getElementById("gatewayIP").value.trim())}&dnsIP=${escape(document.getElementById("dnsIP").value.trim())}`,
                {method: "POST"})
                .then();
            document.getElementById("localIP").value = "";
            document.getElementById("subnetMask").value = "";
            document.getElementById("gatewayIP").value = "";
            document.getElementById("dnsIP").value = "";
            setDhcpForm(true);
        }
        document.getElementById("ssid").value = "";
        document.getElementById("pass").value = "";
    }

    function change_AP_settings() {
        fetch(`${props.API}/api/wifi/set_ap?pass=${escape(document.getElementById("pass_ap").value.trim())}`,
            {method: "POST"})
            .then();
        document.getElementById("pass_ap").value = "";
    }

    let dhcp = <></>;

    if (!dhcpForm) {
        dhcp = <>
            <p><label htmlFor="localIP"><CornerDownRight/> {loc.wifiIP}:</label>
                <input type="text" id="localIP" name="localIP" autoCapitalize="none" value={state.localIP}/>
            </p>
            <p><label htmlFor="subnetMask"><CornerDownRight/> {loc.wifiSub}:</label>
                <input type="text" id="subnetMask" name="subnetMask" autoCapitalize="none" value={state.subnetMask}/>
            </p>
            <p><label htmlFor="gatewayIP"><CornerDownRight/> {loc.wifiGW}:</label>
                <input type="text" id="gatewayIP" name="gatewayIP" autoCapitalize="none" value={state.gatewayIP}/>
            </p>
            <p><label htmlFor="dnsIP"><CornerDownRight/> {loc.wifiDNS}:</label>
                <input type="text" id="dnsIP" name="dnsIP" autoCapitalize="none" value={state.dnsIP}/>
            </p>
        </>;
    }

    const form_sta = <><Form>
        <p><label htmlFor="ssid"><Wifi/> SSID:</label>
            <input type="text" id="ssid" name="ssid" autoCapitalize="none"/>
        </p>
        <p><label htmlFor="pass"><Lock/> Password:</label>
            <input type="password" id="pass" name="pass" autoCapitalize="none"/>
            <label htmlFor="unhide" onClick={() => unHidePass("pass")}> </label>
        </p>
        <p><label htmlFor="dhcp"><Server/> DHCP: (currently {state.dhcp ? "on" : "off"}) </label>
            <input type="checkbox" id="dhcp" name="dhcp" checked={dhcpForm} onClick={() => setDhcpForm(!dhcpForm)}/>
        </p>
        {dhcp}
    </Form>
        <Button onClick={() => {
            change_Wifi_settings();
        }}>{loc.globalSave}</Button>
    </>;

    const form_ap = <><Form>
        <p>
            Minimum password length: <b>8 characters</b>
        </p>
        <p>
            Leave empty to get open access point (without password)
        </p>
        <p><label htmlFor="pass"><Lock/> Password:</label>
            <input type="password" id="pass_ap" name="pass_ap" autoCapitalize="none"/>
            <label htmlFor="unhide" onClick={() => unHidePass("pass_ap")}> </label>
        </p>
    </Form>
        <Button onClick={() => {
            change_AP_settings();
        }}>Save AP password</Button>
    </>;

    let page = <><h2>{loc.titleWifi}</h2>
        <h3>Status</h3></>;

    let connectedTo;
    if (state.apMode === true) {
        connectedTo = "Portal running in AP mode (standalone)";
    } else if (state.apMode === false) {
        connectedTo = <>Connected to {state.ssid} (<a onClick={() => {
            fetch(`${props.API}/api/wifi/forget`, {method: "POST"}).then();
            setState({
                "apMode": true,
                "dhcp": true,
                "ssid": "",
                "localIP": "",
                "subnetMask": "",
                "gatewayIP": "",
                "dnsIP": ""
            });
        }}>Forget</a>)</>;
    }

    page = <>{page}<p>{connectedTo == null ? <Spinner/> : connectedTo}</p></>;
    page = <>{page}<h3>Update station credentials &amp; network settings</h3>{form_sta}</>;
    page = <>{page}<h3>Update access point credentials</h3>{form_ap}</>;

    return page;
}
