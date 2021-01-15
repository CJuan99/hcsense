import { Component, OnInit } from '@angular/core';
import { IMqttMessage, MqttService } from 'ngx-mqtt';
import { Subscription } from 'rxjs';

@Component({
  selector: 'mqtt_dashboard',
  templateUrl: './mqtt_dashboard.component.html',
  styleUrls: ['./mqtt_dashboard.component.css']
})
export class MQTTDashboardComponent implements OnInit {
  private subscription: Subscription;
  topicname: any = 'HCSense/stopDur';
  msg: any;
  publish_msg: any = '10000';

  constructor(private mqttService: MqttService) {
  }

  ngOnInit(){
    // --- Subscribe ---
    this.mqttService.observe('HCSense/temp')
      .subscribe((message: IMqttMessage) => {
        this.msg = message.payload.toString();
      });
  }

  ngOnDestroy(){
    this.subscription.unsubscribe();
  }

  // --- Publish ---
  sendmsg(){
    this.mqttService.unsafePublish(this.topicname, this.publish_msg, { qos: 1, retain: true })
  }

}
