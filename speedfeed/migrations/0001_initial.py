# -*- coding: utf-8 -*-
# Generated by Django 1.9.6 on 2016-05-22 01:24
from __future__ import unicode_literals

from django.db import migrations, models
import django.db.models.deletion
import django_fsm


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Raw',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('timestamp', models.DateTimeField()),
                ('speed', models.FloatField()),
                ('acc_pedal_pos', models.FloatField()),
                ('rpm', models.FloatField()),
                ('yawrate', models.FloatField()),
                ('latitude', models.FloatField()),
                ('longitude', models.FloatField()),
            ],
        ),
        migrations.CreateModel(
            name='RawParticipant',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('vel_x', models.FloatField()),
                ('vel_y', models.FloatField()),
                ('length', models.FloatField()),
                ('height', models.FloatField()),
            ],
        ),
        migrations.CreateModel(
            name='Vehicle',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('top_speed', models.FloatField()),
                ('last_zero_speed_time', models.DateTimeField()),
                ('state', django_fsm.FSMField(default='measuring', max_length=50)),
            ],
        ),
        migrations.CreateModel(
            name='ZeroToHundred',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('time', models.DurationField()),
                ('timestamp', models.DateTimeField()),
                ('vehicle', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='speedfeed.Vehicle')),
            ],
        ),
        migrations.AddField(
            model_name='raw',
            name='vehicle',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='speedfeed.Vehicle'),
        ),
    ]
